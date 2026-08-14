#pragma once

#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace Dns {

// Strict IPv4 validation: exactly four decimal octets, each in [0, 255].
// Rejects Windows-style shorthand forms (e.g. "127.1") on purpose.
bool isValidIpv4(const QString& address);

// IPv6 validation using QHostAddress; accepts compressed forms ("::1").
bool isValidIpv6(const QString& address);

} // namespace Dns

// IPv4 / IPv6 DNS server pair used by a profile or an adapter.
struct DnsConfiguration
{
    QString primaryIpv4;
    QString secondaryIpv4;
    QString primaryIpv6;
    QString secondaryIpv6;

    bool isValid() const;
    bool hasAny() const;

    // Non-empty addresses in apply order (IPv4 primary/secondary, IPv6
    // primary/secondary). What SetInterfaceDnsSettings expects.
    QStringList serversList() const;

    bool operator==(const DnsConfiguration& other) const = default;

    QJsonObject toJson() const;
    static DnsConfiguration fromJson(const QJsonObject& obj);
};

Q_DECLARE_METATYPE(DnsConfiguration)