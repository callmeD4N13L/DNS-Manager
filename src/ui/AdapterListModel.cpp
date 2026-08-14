#include "ui/AdapterListModel.hpp"

#include <QHash>
#include <QVariant>

AdapterListModel::AdapterListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int AdapterListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_adapters.size();
}

QVariant AdapterListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_adapters.size())
        return {};

    const NetworkAdapter& adapter = m_adapters.at(index.row());
    const QString adapterId = adapter.id.toString(QUuid::WithoutBraces);

    switch (role) {
    case IdRole:
        return adapterId;
    case NameRole:
        return adapter.name;
    case FriendlyNameRole:
        return adapter.friendlyName;
    case TypeRole:
        return static_cast<int>(adapter.type);
    case TypeLabelRole:
        return NetworkAdapter::typeToString(adapter.type);
    case EnabledRole:
        return adapter.enabled;
    case ConnectedRole:
        return adapter.connected;
    case DhcpEnabledRole:
        return adapter.dhcpEnabled;
    case Ipv4Role:
        return adapter.ipv4Addresses.join(QStringLiteral(", "));
    case Ipv6Role:
        return adapter.ipv6Addresses.join(QStringLiteral(", "));
    case DnsRole:
        return adapter.dnsServers.join(QStringLiteral(", "));
    case SelectedRole:
        return adapterId == m_selectedId;
    default:
        return {};
    }
}

QHash<int, QByteArray> AdapterListModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { FriendlyNameRole, "friendlyName" },
        { TypeRole, "type" },
        { TypeLabelRole, "typeLabel" },
        { EnabledRole, "enabled" },
        { ConnectedRole, "connected" },
        { DhcpEnabledRole, "dhcpEnabled" },
        { Ipv4Role, "ipv4" },
        { Ipv6Role, "ipv6" },
        { DnsRole, "dns" },
        { SelectedRole, "isSelected" },
    };
}

void AdapterListModel::setAdapters(const QList<NetworkAdapter>& adapters)
{
    beginResetModel();
    m_adapters = adapters;
    endResetModel();
    emit countChanged();
}

void AdapterListModel::setSelectedId(const QString& id)
{
    if (id == m_selectedId)
        return;
    const int previous = selectedRow();
    m_selectedId = id;
    const int current = selectedRow();

    if (previous >= 0)
        emit dataChanged(index(previous), index(previous), { SelectedRole });
    if (current >= 0 && current != previous)
        emit dataChanged(index(current), index(current), { SelectedRole });
}

QVariantMap AdapterListModel::adapterAt(int row) const
{
    if (row < 0 || row >= m_adapters.size())
        return {};
    const NetworkAdapter& adapter = m_adapters.at(row);
    QVariantMap map;
    map.insert(QStringLiteral("id"), adapter.id.toString(QUuid::WithoutBraces));
    map.insert(QStringLiteral("name"), adapter.name);
    map.insert(QStringLiteral("friendlyName"), adapter.friendlyName);
    map.insert(QStringLiteral("typeLabel"), NetworkAdapter::typeToString(adapter.type));
    map.insert(QStringLiteral("enabled"), adapter.enabled);
    map.insert(QStringLiteral("connected"), adapter.connected);
    map.insert(QStringLiteral("dhcpEnabled"), adapter.dhcpEnabled);
    map.insert(QStringLiteral("ipv4"), adapter.ipv4Addresses);
    map.insert(QStringLiteral("ipv6"), adapter.ipv6Addresses);
    map.insert(QStringLiteral("dns"), adapter.dnsServers);
    map.insert(QStringLiteral("isSelected"), adapter.id.toString(QUuid::WithoutBraces) == m_selectedId);
    return map;
}

int AdapterListModel::selectedRow() const
{
    for (int i = 0; i < m_adapters.size(); ++i) {
        if (m_adapters.at(i).id.toString(QUuid::WithoutBraces) == m_selectedId)
            return i;
    }
    return -1;
}
