#include "ui/ProfileListModel.hpp"

#include <QHash>
#include <QJsonObject>
#include <QVariant>

#include "core/ProfileManager.hpp"

ProfileListModel::ProfileListModel(ProfileManager* profiles, QObject* parent)
    : QAbstractListModel(parent)
    , m_profiles(profiles)
{
    connect(m_profiles, &ProfileManager::profilesChanged, this, &ProfileListModel::rebuild);
}

int ProfileListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_filtered.size();
}

QVariant ProfileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtered.size())
        return {};

    const QList<DnsProfile>& list = m_profiles->profiles();
    const DnsProfile& profile = list.at(m_filtered.at(index.row()));
    switch (role) {
    case IdRole:
        return profile.id.toString(QUuid::WithoutBraces);
    case NameRole:
        return profile.name;
    case ProviderRole:
        return profile.provider;
    case PrimaryIpv4Role:
        return profile.primaryIpv4;
    case SecondaryIpv4Role:
        return profile.secondaryIpv4;
    case PrimaryIpv6Role:
        return profile.primaryIpv6;
    case SecondaryIpv6Role:
        return profile.secondaryIpv6;
    case DescriptionRole:
        return profile.description;
    case FavoriteRole:
        return profile.isFavorite;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProfileListModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { ProviderRole, "provider" },
        { PrimaryIpv4Role, "primaryIpv4" },
        { SecondaryIpv4Role, "secondaryIpv4" },
        { PrimaryIpv6Role, "primaryIpv6" },
        { SecondaryIpv6Role, "secondaryIpv6" },
        { DescriptionRole, "description" },
        { FavoriteRole, "favorite" },
    };
}

QString ProfileListModel::addProfile(const QVariantMap& fields)
{
    return m_profiles->addProfile(fields);
}

bool ProfileListModel::updateProfile(const QString& id, const QVariantMap& fields)
{
    return m_profiles->updateProfile(id, fields);
}

bool ProfileListModel::removeProfile(const QString& id)
{
    return m_profiles->removeProfile(id);
}

bool ProfileListModel::toggleFavorite(const QString& id)
{
    return m_profiles->toggleFavorite(id);
}

void ProfileListModel::setSearch(const QString& text)
{
    const QString normalized = text.trimmed().toCaseFolded();
    if (normalized == m_search)
        return;
    m_search = normalized;
    rebuild();
}

QVariantList ProfileListModel::profilesList() const
{
    QVariantList result;
    result.reserve(m_profiles->profiles().size());
    for (const DnsProfile& profile : m_profiles->profiles()) {
        QVariantMap map;
        map.insert(QStringLiteral("id"), profile.id.toString(QUuid::WithoutBraces));
        map.insert(QStringLiteral("name"), profile.name);
        map.insert(QStringLiteral("provider"), profile.provider);
        map.insert(QStringLiteral("description"), profile.description);
        map.insert(QStringLiteral("primaryIpv4"), profile.primaryIpv4);
        map.insert(QStringLiteral("secondaryIpv4"), profile.secondaryIpv4);
        map.insert(QStringLiteral("primaryIpv6"), profile.primaryIpv6);
        map.insert(QStringLiteral("secondaryIpv6"), profile.secondaryIpv6);
        map.insert(QStringLiteral("favorite"), profile.isFavorite);
        result.append(map);
    }
    return result;
}

const QList<DnsProfile>& ProfileListModel::profiles() const
{
    return m_profiles->profiles();
}

void ProfileListModel::rebuild()
{
    m_filtered.clear();
    const QList<DnsProfile>& list = m_profiles->profiles();
    if (m_search.isEmpty()) {
        for (int i = 0; i < list.size(); ++i)
            m_filtered.append(i);
    } else {
        for (int i = 0; i < list.size(); ++i) {
            const DnsProfile& p = list.at(i);
            if (p.name.contains(m_search, Qt::CaseInsensitive)
                || p.provider.contains(m_search, Qt::CaseInsensitive)
                || p.primaryIpv4.contains(m_search, Qt::CaseInsensitive)
                || p.secondaryIpv4.contains(m_search, Qt::CaseInsensitive)
                || p.primaryIpv6.contains(m_search, Qt::CaseInsensitive)
                || p.secondaryIpv6.contains(m_search, Qt::CaseInsensitive)) {
                m_filtered.append(i);
            }
        }
    }

    beginResetModel();
    endResetModel();
    emit countChanged();
}
