#include "core/ProfileManager.hpp"

#include <QFile>
#include <QJsonObject>
#include <QJsonValue>
#include <QUuid>

#include "core/ProfileStorage.hpp"

ProfileManager::ProfileManager(QObject* parent)
    : QObject(parent)
{
}

bool ProfileManager::load()
{
    const QString path = ProfileStorage::filePath();
    const bool fileExists = QFile::exists(path);

    QList<DnsProfile> loaded;
    const OperationResult op = ProfileStorage::load(loaded);
    if (op.success) {
        m_profiles = loaded;
        return true;
    }

    if (!fileExists) {
        seedDefaults();
        persist();
        return true;
    }
    return false; // file exists but could not be read; keep the list empty
}

const QList<DnsProfile>& ProfileManager::profiles() const
{
    return m_profiles;
}

const DnsProfile* ProfileManager::profileById(const QString& id) const
{
    const int index = indexOf(id);
    return index >= 0 ? &m_profiles.at(index) : nullptr;
}

int ProfileManager::indexOf(const QString& id) const
{
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles.at(i).id.toString(QUuid::WithoutBraces) == id)
            return i;
    }
    return -1;
}

QString ProfileManager::addProfile(const QVariantMap& fields)
{
    DnsProfile profile;
    if (!mapToProfile(fields, profile))
        return {};

    m_profiles.append(profile);
    persist();
    emit profilesChanged();
    return profile.id.toString(QUuid::WithoutBraces);
}

bool ProfileManager::updateProfile(const QString& id, const QVariantMap& fields)
{
    const int row = indexOf(id);
    if (row < 0)
        return false;

    DnsProfile updated;
    if (!mapToProfile(fields, updated))
        return false;
    updated.id = m_profiles.at(row).id; // ids are immutable

    m_profiles[row] = updated;
    persist();
    emit profilesChanged();
    return true;
}

bool ProfileManager::removeProfile(const QString& id)
{
    const int row = indexOf(id);
    if (row < 0)
        return false;

    m_profiles.removeAt(row);
    persist();
    emit profilesChanged();
    return true;
}

bool ProfileManager::toggleFavorite(const QString& id)
{
    const int row = indexOf(id);
    if (row < 0)
        return false;

    m_profiles[row].isFavorite = !m_profiles.at(row).isFavorite;
    persist();
    emit profilesChanged();
    return true;
}

bool ProfileManager::importFromFile(const QString& path)
{
    QList<DnsProfile> incoming;
    const OperationResult op = ProfileStorage::loadFromFile(path, incoming);
    if (!op.success)
        return false;

    int added = 0;
    for (const DnsProfile& profile : incoming) {
        if (indexOf(profile.id.toString(QUuid::WithoutBraces)) >= 0)
            continue;
        m_profiles.append(profile);
        ++added;
    }
    if (added == 0)
        return true; // nothing new, but not an error

    persist();
    emit profilesChanged();
    return true;
}

bool ProfileManager::exportToFile(const QString& path)
{
    return ProfileStorage::saveToFile(path, m_profiles).success;
}

void ProfileManager::resetToDefaults()
{
    m_profiles.clear();
    seedDefaults();
    persist();
    emit profilesChanged();
}

void ProfileManager::seedDefaults()
{
    const QList<QJsonObject> samples = {
        {
            { "name", "Cloudflare" },
            { "provider", "Cloudflare, Inc." },
            { "primary_ipv4", "1.1.1.1" },
            { "secondary_ipv4", "1.0.0.1" },
            { "primary_ipv6", "2606:4700:4700::1111" },
            { "secondary_ipv6", "2606:4700:4700::1001" },
            { "favorite", true },
        },
        {
            { "name", "Google" },
            { "provider", "Google LLC" },
            { "primary_ipv4", "8.8.8.8" },
            { "secondary_ipv4", "8.8.4.4" },
            { "primary_ipv6", "2001:4860:4860::8888" },
            { "secondary_ipv6", "2001:4860:4860::8844" },
        },
        {
            { "name", "Quad9" },
            { "provider", "Quad9 Foundation" },
            { "primary_ipv4", "9.9.9.9" },
            { "secondary_ipv4", "149.112.112.112" },
        },
        {
            { "name", "AdGuard" },
            { "provider", "AdGuard Software Ltd." },
            { "primary_ipv4", "94.140.14.14" },
            { "secondary_ipv4", "94.140.15.15" },
            { "primary_ipv6", "2a10:50c0::ad1:ff" },
            { "secondary_ipv6", "2a10:50c0::ad2:ff" },
        },
    };

    for (const QJsonObject& obj : samples)
        m_profiles.append(DnsProfile::fromJson(obj));
}

bool ProfileManager::persist() const
{
    const OperationResult op = ProfileStorage::save(m_profiles);
    if (!op.success)
        emit const_cast<ProfileManager*>(this)->storageError(op.message);
    return op.success;
}

bool ProfileManager::mapToProfile(const QVariantMap& fields, DnsProfile& out)
{
    out.name = fields.value(QStringLiteral("name")).toString().trimmed();
    out.provider = fields.value(QStringLiteral("provider")).toString();
    out.description = fields.value(QStringLiteral("description")).toString();
    out.primaryIpv4 = fields.value(QStringLiteral("primaryIpv4")).toString();
    out.secondaryIpv4 = fields.value(QStringLiteral("secondaryIpv4")).toString();
    out.primaryIpv6 = fields.value(QStringLiteral("primaryIpv6")).toString();
    out.secondaryIpv6 = fields.value(QStringLiteral("secondaryIpv6")).toString();
    out.isFavorite = fields.value(QStringLiteral("favorite")).toBool();
    return out.isValid();
}
