#pragma once

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QUuid>

// A network adapter detected on the system (filled in by Phase 6).
class NetworkAdapter
{
public:
    enum class Type {
        Ethernet,
        Wifi,
        Virtual,
        Vpn,
        Loopback,
        Other
    };

    QUuid id;
    QString name;
    QString friendlyName;
    Type type = Type::Other;
    bool enabled = false;
    bool connected = false;
    bool dhcpEnabled = false;
    QStringList ipv4Addresses;
    QStringList ipv6Addresses;
    QStringList dnsServers;

    static QString typeToString(Type type);

    bool operator==(const NetworkAdapter& other) const = default;
};

Q_DECLARE_METATYPE(NetworkAdapter)