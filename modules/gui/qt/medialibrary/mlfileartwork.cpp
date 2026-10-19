// SPDX-License-Identifier: GPL-2.0-or-later
#include "mlfileartwork.hpp"
#include "medialib.hpp"
#include "mlmediastore.hpp"
#include "maininterface/mainctx.hpp"
#include "qt.hpp"
#include <vlc_preparser.h>
#include <vlc_input_item.h>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>
#include <QTimer>

MLFileArtwork::ExportedFile::~ExportedFile()
{
    if (!imported) QFile::remove(path);
}

MLFileArtwork::MLFileArtwork(QObject* parent) : QObject(parent) {}
MLFileArtwork::~MLFileArtwork()
{
    clear();
    if (m_preparser) vlc_preparser_Delete(m_preparser);
}

void MLFileArtwork::setCtx(MainCtx* ctx)
{
    if (m_ctx == ctx) return;
    clear();
    if (m_preparser) vlc_preparser_Delete(m_preparser);
    m_preparser = nullptr;
    m_store.reset();
    m_metadataStore.reset();
    m_ctx = ctx;
    m_ml = ctx ? ctx->getMediaLibrary() : nullptr;
    if (m_ml) {
        m_store = std::make_unique<MLMediaStore>(m_ml);
        m_metadataStore = std::make_unique<MLMediaStore>(m_ml);
        m_store->setReadAudioMetadata(true);
        m_metadataStore->setReadAudioMetadata(true);
        connect(m_store.get(), &MLMediaStore::updated, this, &MLFileArtwork::mediaUpdated);
        connect(m_metadataStore.get(), &MLMediaStore::updated, this, [this](const QString& mrl, const MLMedia& media) {
            if (m_stateRequested.contains(mrl))
                emit mediaPlayedChanged(QUrl::fromEncoded(mrl.toUtf8()), media.valid() && media.playCount() > 0);
        });
        for (const auto store : {m_store.get(), m_metadataStore.get()})
            connect(store, &MLMediaStore::audioMetadataUpdated, this,
                [this](const QString& mrl, const QString& title, const QString& artist) {
                    if (m_requested.contains(mrl) || m_stateRequested.contains(mrl))
                        emit mediaMetadataChanged(QUrl::fromEncoded(mrl.toUtf8()), title, artist);
                });
    }
    emit ctxChanged();
}

void MLFileArtwork::clear()
{
    ++m_generation;
    if (m_request) {
        vlc_preparser_Cancel(m_preparser, m_request);
        vlc_preparser_req_Release(m_request);
        m_request = nullptr;
    }
    m_pending.clear();
    m_requested.clear();
    m_stateRequested.clear();
    m_attempted.clear();
    if (m_store) m_store->clear();
    if (m_metadataStore) m_metadataStore->clear();
}

void MLFileArtwork::request(const QUrl& url, bool generateVideo)
{
    if (!m_store || !url.isLocalFile()) return;
    const auto mrl = url.toString(QUrl::FullyEncoded);
    const auto previous = m_requested.constFind(mrl);
    if (previous != m_requested.cend() && (previous.value() || !generateVideo)) return;
    m_requested.insert(mrl, generateVideo);
    // External media records cache previews without adding or scanning a folder.
    m_store->insert(mrl, true);
}

void MLFileArtwork::mediaUpdated(const QString& mrl, const MLMedia& media)
{
    if (!m_requested.contains(mrl) && !m_stateRequested.contains(mrl)) return;
    emit mediaPlayedChanged(QUrl::fromEncoded(mrl.toUtf8()), media.valid() && media.playCount() > 0);
    if (!m_requested.contains(mrl)) return;
    emit mediaProgressChanged(QUrl::fromEncoded(mrl.toUtf8()), media.valid() ? media.progress() : 0.0);
    const QString artwork = media.valid() ? media.smallCover() : QString {};
    if (!artwork.isEmpty())
        emit artworkChanged(QUrl::fromEncoded(mrl.toUtf8()), artwork);
    if (!media.valid()) return;
    const bool audio = !m_requested[mrl] || media.type() == VLC_ML_MEDIA_TYPE_AUDIO;
    if (!artwork.isEmpty() && (!audio || media.type() == VLC_ML_MEDIA_TYPE_AUDIO)) return;
    vlc_ml_thumbnail_status_t status;
    media.smallCover(&status);
    if (!audio && status != VLC_ML_THUMBNAIL_STATUS_MISSING && status != VLC_ML_THUMBNAIL_STATUS_FAILURE) return;
    const auto id = media.getId().id;
    if (m_attempted.contains(id)) return;
    m_attempted.insert(id);
    if (audio) {
        m_pending.push_back({mrl, nullptr, id, m_generation.load(), media.type() != VLC_ML_MEDIA_TYPE_AUDIO});
        startNext();
        return;
    }
    const auto directory = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
            .filePath(QStringLiteral("folder-thumbnails"));
    auto file = std::make_shared<ExportedFile>();
    file->path = QDir(directory).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".jpg"));
    const Work work {mrl, file, id, m_generation.load()};
    struct Setup { bool ready = false; };
    m_ml->runOnMLThread<Setup>(this, [directory](vlc_medialibrary_t*, Setup& result) {
        result.ready = QDir().mkpath(directory);
    }, [this, work](quint64, Setup& result) {
        if (work.generation != m_generation || !result.ready) return;
        m_pending.push_back(work);
        startNext();
    });
}

void MLFileArtwork::requestWatchStates(const QList<QUrl>& urls, quint64 serial)
{
    requestMetadataBatch(urls, serial, false);
}

void MLFileArtwork::requestAudioMetadata(const QList<QUrl>& urls, quint64 serial)
{
    requestMetadataBatch(urls, serial, true);
}

void MLFileArtwork::requestMetadataBatch(const QList<QUrl>& urls, quint64 serial, bool audio)
{
    const auto ready = [this, serial, audio, generation = m_generation.load()] {
        if (generation != m_generation) return;
        if (audio) emit audioMetadataReady(serial);
        else emit watchStatesReady(serial);
    };
    if (!m_metadataStore) {
        QTimer::singleShot(0, this, ready);
        return;
    }
    QStringList mrls;
    mrls.reserve(urls.size());
    for (const auto& url : urls) {
        if (!url.isLocalFile()) continue;
        const auto mrl = url.toString(QUrl::FullyEncoded);
        m_stateRequested.insert(mrl);
        mrls.append(mrl);
    }
    // Keep metadata reads separate from visible artwork lookups and external
    // record creation, so a batch cannot invalidate those in-flight results.
    m_metadataStore->insertMany(mrls, ready);
}

void MLFileArtwork::cancelWatchStates()
{
    m_stateRequested.clear();
    if (m_metadataStore) m_metadataStore->cancelBatch();
}

void MLFileArtwork::startNext()
{
    if (m_request || m_pending.empty() || !m_ctx) return;
    const auto work = m_pending.front();
    msg_Dbg(m_ctx->getIntf(), "Folder preview request: %s", qtu(work.mrl));
    m_pending.pop_front();
    if (!m_preparser) {
        vlc_preparser_cfg cfg {};
        cfg.types = VLC_PREPARSER_TYPE_THUMBNAIL_TO_FILES | VLC_PREPARSER_TYPE_PARSE
                | VLC_PREPARSER_TYPE_FETCHMETA_LOCAL;
        cfg.max_parser_threads = 1;
        cfg.max_thumbnailer_threads = 1;
        cfg.timeout = VLC_TICK_FROM_SEC(15);
        cfg.external_process = false;
        m_preparser = vlc_preparser_New(VLC_OBJECT(m_ctx->getIntf()), &cfg);
    }
    struct Request { MLFileArtwork* owner; Work work; };
    static const vlc_thumbnailer_to_files_cbs callbacks = {
        [](vlc_preparser_req*, int status, const bool* results, size_t count, void* opaque) {
            std::unique_ptr<Request> request(static_cast<Request*>(opaque));
            const bool success = status == VLC_SUCCESS && count == 1 && results && results[0];
            if (!success || request->work.generation != request->owner->m_generation)
                QFile::remove(request->work.file->path);
            if (request->work.generation != request->owner->m_generation) return;
            QMetaObject::invokeMethod(request->owner, [owner = request->owner, work = request->work, success] {
                if (work.generation != owner->m_generation) { QFile::remove(work.file->path); return; }
                owner->finishRequest(work, success
                    ? QUrl::fromLocalFile(work.file->path).toString(QUrl::FullyEncoded) : QString {});
            });
        }
    };
    static const vlc_preparser_cbs audioCallbacks = {
        [](vlc_preparser_req* handle, int status, void* opaque) {
            std::unique_ptr<Request> request(static_cast<Request*>(opaque));
            if (request->work.generation != request->owner->m_generation) return;
            QString artwork, title, artist;
            if (status == VLC_SUCCESS) {
                const auto item = vlc_preparser_req_GetItem(handle);
                char* uri = input_item_GetArtURL(item);
                const auto url = QUrl::fromEncoded(QByteArray(uri ? uri : ""));
                free(uri);
                if (url.isLocalFile()) artwork = url.toString(QUrl::FullyEncoded);
                if (request->work.readMetadata) {
                    char* value = input_item_GetTitle(item);
                    title = qfu(value); free(value);
                    value = input_item_GetArtist(item);
                    artist = qfu(value); free(value);
                }
            }
            QMetaObject::invokeMethod(request->owner,
                [owner = request->owner, work = request->work, artwork, title, artist, status] {
                    if (work.generation == owner->m_generation) {
                        if (work.readMetadata && status == VLC_SUCCESS)
                            emit owner->mediaMetadataChanged(QUrl::fromEncoded(work.mrl.toUtf8()), title, artist);
                        owner->finishRequest(work, artwork);
                    }
                });
        }, nullptr, nullptr
    };
    input_item_t* item = input_item_New(qtu(work.mrl), nullptr);
    if (m_preparser && item) {
        auto request = std::make_unique<Request>(Request {this, work});
        if (!work.file) {
            // Read tags and nearby cover files without an online artwork search.
            m_request = vlc_preparser_req_NewParse(m_preparser, item,
                VLC_PREPARSER_TYPE_PARSE | VLC_PREPARSER_TYPE_FETCHMETA_LOCAL,
                &audioCallbacks, request.get());
        } else {
            vlc_thumbnailer_arg args {};
            args.seek.type = decltype(args.seek)::VLC_THUMBNAILER_SEEK_POS;
            args.seek.pos = .15;
            args.seek.speed = decltype(args.seek)::VLC_THUMBNAILER_SEEK_FAST;
            vlc_thumbnailer_output output {};
            output.format = VLC_THUMBNAILER_FORMAT_JPEG;
            output.width = 512; output.height = 320; output.crop = true;
            const auto path = work.file->path.toUtf8();
            output.file_path = path.constData(); output.creat_mode = 0600;
            m_request = vlc_preparser_req_NewThumbnailToFiles(m_preparser, item, &args, &output, 1, &callbacks, request.get());
        }
        if (m_request) {
            Request* context = request.release();
            if (vlc_preparser_Submit(m_preparser, m_request) != VLC_SUCCESS) {
                delete context;
                vlc_preparser_req_Release(m_request);
                m_request = nullptr;
            }
        }
    }
    if (item) input_item_Release(item);
    if (!m_request) QTimer::singleShot(0, this, &MLFileArtwork::startNext);
}

void MLFileArtwork::finishRequest(const Work& work, const QString& artwork)
{
    vlc_preparser_req_Release(m_request);
    m_request = nullptr;
    if (!artwork.isEmpty()) storeArtwork(work, artwork);
    startNext();
}

void MLFileArtwork::storeArtwork(const Work& work, const QString& artwork)
{
    struct Result { QString artwork; };
    m_ml->runOnMLThread<Result>(this, [work, artwork](vlc_medialibrary_t* ml, Result& result) {
        ml_unique_ptr<vlc_ml_media_t> media(vlc_ml_get_media(ml, work.id));
        if (media) {
            // Keep artwork that became available while this decode was running.
            result.artwork = MLMedia(media.get()).smallCover();
            if (result.artwork.isEmpty()) {
                vlc_ml_media_set_thumbnail(ml, work.id, qtu(artwork), VLC_ML_THUMBNAIL_SMALL);
                media.reset(vlc_ml_get_media(ml, work.id));
                if (media) result.artwork = MLMedia(media.get()).smallCover();
            }
        }
        if (work.file)
            work.file->imported = QUrl::fromEncoded(result.artwork.toUtf8()) == QUrl::fromLocalFile(work.file->path);
    }, [this, work](quint64, Result& result) {
        if (!result.artwork.isEmpty() && work.generation == m_generation && m_requested.contains(work.mrl))
            emit artworkChanged(QUrl::fromEncoded(work.mrl.toUtf8()), result.artwork);
    });
}
