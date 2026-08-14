#include "core/ProfileStorage.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace {

QString defaultFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + QLatin1Char('/') + QStringLiteral("dns_profiles.json");
}

} // namespace

QString ProfileStorage::filePath()
{
    return defaultFilePath();
}

OperationResult ProfileStorage::load(QList<DnsProfile>& profiles)
{
    if (!QFile::exists(filePath())) {
        profiles.clear(); // nothing to load resets the output (no stale data)
        return OperationResult::ok(); // no file yet = nothing to load
    }
    return loadFromFile(filePath(), profiles);
}

OperationResult ProfileStorage::loadFromFile(const QString& path, QList<DnsProfile>& profiles)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return OperationResult::fail(
            QObject::tr("Could not open profile file: %1").arg(file.errorString()));

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (doc.isNull() || parseError.error != QJsonParseError::NoError)
        return OperationResult::fail(
            QObject::tr("Profile file is not valid JSON: %1").arg(parseError.errorString()));

    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("version")).toInt() != kFileVersion)
        return OperationResult::fail(QObject::tr("Unsupported profile file version"));

    QList<DnsProfile> result;
    const QJsonArray array = root.value(kProfileArrayKey).toArray();
    for (const QJsonValue& value : array) {
        if (!value.isObject())
            continue;
        const DnsProfile profile = DnsProfile::fromJson(value.toObject());
        if (profile.isValid())
            result.append(profile);
    }

    profiles = result;
    return OperationResult::ok();
}

OperationResult ProfileStorage::save(const QList<DnsProfile>& profiles)
{
    return saveToFile(filePath(), profiles);
}

OperationResult ProfileStorage::saveToFile(const QString& path, const QList<DnsProfile>& profiles)
{
    QJsonArray array;
    for (const DnsProfile& profile : profiles)
        array.append(profile.toJson());

    QJsonObject root;
    root.insert(QStringLiteral("version"), kFileVersion);
    root.insert(kProfileArrayKey, array);

    QDir().mkpath(QFileInfo(path).absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return OperationResult::fail(
            QObject::tr("Could not write profile file: %1").arg(file.errorString()));

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit())
        return OperationResult::fail(
            QObject::tr("Could not save profile file: %1").arg(file.errorString()));
    return OperationResult::ok();
}
