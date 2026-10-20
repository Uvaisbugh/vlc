// SPDX-License-Identifier: GPL-2.0-or-later
#include "release_info.hpp"

#include <QJsonObject>
#include <QRegularExpression>
#include <QSysInfo>
#include <cmath>
#include <limits>

namespace vlc::updates {

std::optional<Version> Version::parse(QString text)
{
    // Read old published tags without moving or renaming those releases.
    static const QRegularExpression legacy(QStringLiteral("^beta-([1-9][0-9]*)$"));
    const auto old = legacy.match(text);
    if (old.hasMatch())
        text = QStringLiteral("4.0.0-beta.") + old.captured(1);
    static const QRegularExpression pattern(QStringLiteral(
        "^v?(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)"
        "(?:-([0-9A-Za-z-]+(?:\\.[0-9A-Za-z-]+)*))?"
        "(?:\\+([0-9A-Za-z-]+(?:\\.[0-9A-Za-z-]+)*))?$"));
    const auto match = pattern.match(text);
    if (!match.hasMatch())
        return std::nullopt;
    Version result;
    unsigned* parts[] = {&result.major, &result.minor, &result.patch};
    for (int i = 0; i < 3; ++i)
    {
        bool ok = false;
        *parts[i] = match.captured(i + 1).toUInt(&ok);
        if (!ok || *parts[i] > unsigned(std::numeric_limits<int>::max()))
            return std::nullopt;
    }
    if (!match.captured(4).isEmpty())
        result.prerelease = match.captured(4).split(QLatin1Char('.'));
    static const QRegularExpression numeric(QStringLiteral("^[0-9]+$"));
    for (const auto& part : result.prerelease)
        if (numeric.match(part).hasMatch() && part.size() > 1 && part.startsWith(QLatin1Char('0')))
            return std::nullopt;
    result.text = text.startsWith(QLatin1Char('v')) ? text.mid(1) : text;
    return result;
}

int Version::compare(const Version& other) const
{
    const unsigned a[] = {major, minor, patch};
    const unsigned b[] = {other.major, other.minor, other.patch};
    for (int i = 0; i < 3; ++i)
        if (a[i] != b[i])
            return a[i] < b[i] ? -1 : 1;
    if (isPrerelease() != other.isPrerelease())
        return isPrerelease() ? -1 : 1;
    static const QRegularExpression numeric(QStringLiteral("^[0-9]+$"));
    for (qsizetype i = 0; i < qMin(prerelease.size(), other.prerelease.size()); ++i)
    {
        const auto& left = prerelease[i];
        const auto& right = other.prerelease[i];
        if (left == right)
            continue;
        const bool ln = numeric.match(left).hasMatch();
        const bool rn = numeric.match(right).hasMatch();
        if (ln != rn)
            return ln ? -1 : 1;
        // Compare arbitrary-length numeric identifiers without integer overflow.
        if (ln && left.size() != right.size())
            return left.size() < right.size() ? -1 : 1;
        return QString::compare(left, right, Qt::CaseSensitive) < 0 ? -1 : 1;
    }
    return prerelease.size() == other.prerelease.size() ? 0
         : prerelease.size() < other.prerelease.size() ? -1 : 1;
}

Target Target::current()
{
    Target target;
#if defined(Q_OS_WIN)
    target.os = QStringLiteral("windows");
#elif defined(Q_OS_MACOS)
    target.os = QStringLiteral("macos");
#elif defined(Q_OS_LINUX)
    target.os = QStringLiteral("linux");
#endif
    target.arch = QSysInfo::buildCpuArchitecture();
    if (target.arch == QLatin1String("aarch64"))
        target.arch = QStringLiteral("arm64");
    else if (target.arch == QLatin1String("i386"))
        target.arch = QStringLiteral("x86");
    return target;
}

QStringList Target::packageSuffixes() const
{
    if (os == QLatin1String("windows"))
    {
        if (arch == QLatin1String("x86_64")) return {QStringLiteral("win64.zip")};
        if (arch == QLatin1String("arm64")) return {QStringLiteral("winarm64.zip")};
        if (arch == QLatin1String("x86")) return {QStringLiteral("win32.zip")};
    }
    else if (os == QLatin1String("macos") &&
             (arch == QLatin1String("x86_64") || arch == QLatin1String("arm64")))
        return {QStringLiteral("macos-") + arch + QStringLiteral(".zip"),
                QStringLiteral("macos-universal.zip")};
    else if (os == QLatin1String("linux") &&
             (arch == QLatin1String("x86_64") || arch == QLatin1String("arm64")))
        return {QStringLiteral("linux-") + arch + QStringLiteral(".tar.gz")};
    return {};
}

static bool githubUrl(const QUrl& url, const QString& path)
{
    return url.isValid() && url.scheme() == QLatin1String("https") &&
           url.host() == QLatin1String("github.com") && url.port(-1) == -1 &&
           url.userInfo().isEmpty() && url.query().isEmpty() && url.fragment().isEmpty() &&
           url.path() == path;
}

std::optional<Release> selectRelease(const QJsonArray& releases,
                                    const Version& installed, const Target& target)
{
    std::optional<Release> best;
    const QString repo = QStringLiteral("/dsojevic/juxtopposed-vlc/releases/");
    for (const auto& entry : releases)
    {
        const auto object = entry.toObject();
        if (!object.value(QStringLiteral("draft")).isBool() ||
            object.value(QStringLiteral("draft")).toBool() ||
            !object.value(QStringLiteral("prerelease")).isBool())
            continue;
        const auto tag = object.value(QStringLiteral("tag_name")).toString();
        const auto version = Version::parse(tag);
        if (!version || version->compare(installed) <= 0 ||
            (best && version->compare(best->version) <= 0))
            continue;
        const bool prerelease = object.value(QStringLiteral("prerelease")).toBool();
        if (prerelease != version->isPrerelease() || (!installed.isPrerelease() && prerelease))
            continue;
        const QUrl page(object.value(QStringLiteral("html_url")).toString());
        if (!githubUrl(page, repo + QStringLiteral("tag/") + tag))
            continue;
        const auto assets = object.value(QStringLiteral("assets")).toArray();
        for (const auto& suffix : target.packageSuffixes())
        {
            const QString expected = QStringLiteral("vlc-juxtopposed-") + tag + QLatin1Char('-') + suffix;
            for (const auto& assetEntry : assets)
            {
                const auto asset = assetEntry.toObject();
                const auto size = asset.value(QStringLiteral("size")).toDouble();
                const QUrl url(asset.value(QStringLiteral("browser_download_url")).toString());
                if (asset.value(QStringLiteral("name")).toString() != expected ||
                    asset.value(QStringLiteral("state")).toString() != QLatin1String("uploaded") ||
                    size <= 0 || size > 9007199254740991.0 || std::floor(size) != size ||
                    !githubUrl(url, repo + QStringLiteral("download/") + tag + QLatin1Char('/') + expected))
                    continue;
                best = Release{*version, tag, object.value(QStringLiteral("body")).toString(), page,
                               {expected, url, qint64(size), asset.value(QStringLiteral("digest")).toString()}};
                break;
            }
            if (best && best->tag == tag)
                break;
        }
    }
    return best;
}

} // namespace vlc::updates
