// SPDX-License-Identifier: GPL-2.0-or-later
#include "config.h"
#include <QTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QAtomicInt>
#include <QTemporaryDir>
#include <QPainter>
#include "../util/effects_image_provider.hpp"

class CountingEffectsProvider final : public EffectsImageProvider
{
public:
    using EffectsImageProvider::EffectsImageProvider;
    QAtomicInt requests{0};
    QAtomicInt requestedWidth{0};
    QAtomicInt requestedHeight{0};
    QImage requestImage(const QString& id, QSize* size, const QSize& requested) override
    {
        requests.fetchAndAddRelaxed(1);
        requestedWidth.storeRelaxed(requested.width());
        requestedHeight.storeRelaxed(requested.height());
        return EffectsImageProvider::requestImage(id, size, requested);
    }
};

class EffectsImageProviderTest : public QObject
{
    Q_OBJECT
    static QVariantMap properties()
    {
        return {{"viewportWidth", 222}, {"viewportHeight", 182},
            {"rectWidth", 160}, {"rectHeight", 120}, {"yOffset", 8},
            {"blurRadius", 15}, {"color", QColor(0, 0, 0, 117)},
            {"xRadius", 10}, {"yRadius", 10},
            {"exclusionX", 31}, {"exclusionY", 31},
            {"exclusionWidth", 160}, {"exclusionHeight", 120}, {"exclusionRadius", 10}};
    }
private slots:
    void initTestCase()
    {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
        qmlRegisterSingletonType<EffectsImageProvider>("VLC.Util", 1, 0, "Effects", &EffectsImageProvider::instance);
    }
    void keepsTranslucentSurfaceClear_data()
    {
        QTest::addColumn<int>("density");
        QTest::newRow("1x") << 1;
        QTest::newRow("2x") << 2;
    }
    void keepsTranslucentSurfaceClear()
    {
        QFETCH(int, density);
        QQmlEngine engine;
        auto* provider = new EffectsImageProvider(&engine);
        auto settings = properties();
        auto render = [&] {
            const auto url = EffectsImageProvider::url(EffectsImageProvider::RoundedRectDropShadow, settings);
            QSize size;
            return provider->requestImage(url.toString().mid(QStringLiteral("image://effects/").length()), &size, QSize(222, 182) * density);
        };
        const auto image = render();
        QVERIFY(!image.isNull());
        QCOMPARE(image.size(), QSize(222, 182) * density);
        QCOMPARE(image.pixelColor(110 * density, 90 * density).alpha(), 0);
        QCOMPARE(image.pixelColor(110 * density, 149 * density).alpha(), 0);
        QVERIFY(image.pixelColor(110 * density, 155 * density).alpha() > 0);
        QVERIFY(image.pixelColor(32 * density, 149 * density).alpha() > 0); // Outside the rounded card corner.
        QVERIFY(image.pixelColor(110 * density, 157 * density).alpha() > image.pixelColor(110 * density, 25 * density).alpha());
        settings.remove("exclusionWidth");
        const auto solid = render();
        QVERIFY(solid.pixelColor(110 * density, 90 * density).alpha() > 100);
        QCOMPARE(solid.pixelColor(110 * density, 155 * density), image.pixelColor(110 * density, 155 * density));
    }
    void rendersInSoftware()
    {
        QQuickView view;
        auto* provider = new CountingEffectsProvider(view.engine());
        QQmlComponent component(view.engine());
        component.setData("import QtQuick\nimport \".\" as Redesign\n"
            "Rectangle { width: 400; height: 300; color: \"white\"; property bool shadowOpen: true\n"
            "Loader { x: 80; y: 50; width: 160; height: 120; active: parent.shadowOpen\n"
            "sourceComponent: Component { Redesign.RedesignSoftwareShadow { cornerRadius: 10 } } }\n"
            "Rectangle { x: 80; y: 50; width: 160; height: 120; radius: 10; color: \"#80191919\" }\n}",
            QUrl::fromLocalFile(QStringLiteral(TOP_SRCDIR "/modules/gui/qt/player/qml/redesign/")));
        QVERIFY2(!component.isError(), qPrintable(component.errorString()));
        auto* root = qobject_cast<QQuickItem*>(component.create());
        QVERIFY2(root, qPrintable(component.errorString()));
        view.setContent(QUrl(), &component, root);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        auto* image = root->findChild<QObject*>("softwareMenuShadowImage");
        QVERIFY(image);
        QTRY_COMPARE(image->property("status").toInt(), 1);
        QCOMPARE(provider->requestedWidth.loadRelaxed(), qRound(222 * view.devicePixelRatio()));
        QCOMPARE(provider->requestedHeight.loadRelaxed(), qRound(182 * view.devicePixelRatio()));
        QTest::qWait(50);
        const auto pixels = view.grabWindow();
        QVERIFY(!pixels.isNull());
        const auto density = pixels.width() / 400.;
        QCOMPARE(pixels.pixelColor(qRound(160 * density), qRound(100 * density)), QColor(140, 140, 140));
        QVERIFY(pixels.pixelColor(qRound(160 * density), qRound(175 * density)).red() < 245);
        QCOMPARE(pixels.pixelColor(qRound(20 * density), qRound(20 * density)), QColor(Qt::white));
        const auto output = qEnvironmentVariable("VLC_TEST_SHADOW_OUTPUT");
        if (!output.isEmpty()) QVERIFY(pixels.save(output));
        const auto requests = provider->requests.loadRelaxed();
        QVERIFY(requests > 0);
        root->setProperty("shadowOpen", false);
        QTRY_VERIFY(!root->findChild<QObject*>("softwareMenuShadowImage"));
        root->setProperty("shadowOpen", true);
        image = root->findChild<QObject*>("softwareMenuShadowImage");
        QVERIFY(image);
        QTRY_COMPARE(image->property("status").toInt(), 1);
        QCOMPARE(provider->requests.loadRelaxed(), requests);
    }
    void artworkKeepsPixelsAndRoundsCorners()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        const QString path = folder.filePath("cover with spaces %.png");
        QImage source(200, 100, QImage::Format_RGB32);
        source.fill(Qt::red);
        QPainter painter(&source);
        painter.fillRect(100, 0, 100, 100, Qt::blue);
        painter.end();
        QVERIFY(source.save(path));
        QQmlEngine engine;
        auto* provider = new EffectsImageProvider(&engine);
        auto render = [&](QVariantMap settings, QSize requested) {
            settings.insert("source", QUrl::fromLocalFile(path));
            const auto url = EffectsImageProvider::url(EffectsImageProvider::Artwork, settings);
            QSize size;
            return provider->requestImage(url.toString().mid(QStringLiteral("image://effects/").length()), &size, requested);
        };
        const auto rounded = render({{"radius", .1}}, QSize(400, 400));
        QCOMPARE(rounded.size(), QSize(400, 200));
        QCOMPARE(rounded.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(rounded.pixelColor(50, 100), QColor(Qt::red));
        QCOMPARE(rounded.pixelColor(350, 100), QColor(Qt::blue));
        const auto blurred = render({{"blur", .05}}, QSize(100000, 100000));
        QCOMPARE(blurred.size(), QSize(512, 256));
        QVERIFY(blurred.pixelColor(255, 128).red() > 30);
        QVERIFY(blurred.pixelColor(255, 128).blue() > 30);
        QCOMPARE(render({}, QSize(100000, 100000)).size(), QSize(2048, 1024));
    }
    void artworkRejectsMissingAndNonlocalSources()
    {
        QQmlEngine engine;
        auto* provider = new EffectsImageProvider(&engine);
        for (const auto& source : {"file:///missing-cover.png", "https://127.0.0.1:9/cover.png", "file://server/share/cover.png"}) {
            const auto url = EffectsImageProvider::url(EffectsImageProvider::Artwork, {{"source", QUrl(source)}});
            QSize size;
            QVERIFY(provider->requestImage(url.toString().mid(QStringLiteral("image://effects/").length()), &size, QSize(512, 512)).isNull());
            QVERIFY(size.isEmpty());
        }
    }
};
QTEST_MAIN(EffectsImageProviderTest)
#include "test_effects_image_provider.moc"
