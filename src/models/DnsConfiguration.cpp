#include "models/DnsConfiguration.hpp"

#include <QAbstractSocket>
#include <QHostAddress>
#include <QRegularExpression>

namespace Dns {

bool isValidIpv4(const QString& address)
{
    static const QRegularExpression re(
        QStringLiteral(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)"));
    const QRegularExpressionMatch match = re.match(address);
    if (!match.hasMatch())
        return false;
    for (int i = 1; i <= 4; ++i) {
        const int octet = match.captured(i).toInt();
        if (octet > 255)
            return false;
    }
    return true;
}

bool isValidIpv6(const QString& address)
{
    if (address.isEmpty())
        return false;
    const QHostAddress parsed(address);
    return parsed.protocol() == QAbstractSocket::IPv6Protocol;
}

} // namespace Dns

bool DnsConfiguration::isValid() const
{
    return (primaryIpv4.isEmpty() || Dns::isValidIpv4(primaryIpv4))
        && (secondaryIpv4.isEmpty() || Dns::isValidIpv4(secondaryIpv4))
        && (primaryIpv6.isEmpty() || Dns::isValidIpv6(primaryIpv6))
        && (secondaryIpv6.isEmpty() || Dns::isValidIpv6(secondaryIpv6));
}

bool DnsConfiguration::hasAny() const
{
    return !primaryIpv4.isEmpty() || !secondaryIpv4.isEmpty()
        || !primaryIpv6.isEmpty() || !secondaryIpv6.isEmpty();
}

QStringList DnsConfiguration::serversList() const
{
    QStringList list;
    for (const QString& address : { primaryIpv4, secondaryIpv4, primaryIpv6, secondaryIpv6 }) {
        if (!address.isEmpty())
            list.append(address);
    }
    return list;
}

QJsonObject DnsConfiguration::toJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("primary_ipv4"), primaryIpv4);
    obj.insert(QStringLiteral("secondary_ipv4"), secondaryIpv4);
    obj.insert(QStringLiteral("primary_ipv6"), primaryIpv6);
    obj.insert(QStringLiteral("secondary_ipv6"), secondaryIpv6);
    return obj;
}

DnsConfiguration DnsConfiguration::fromJson(const QJsonObject& obj)
{
    DnsConfiguration cfg;
    cfg.primaryIpv4 = obj.value(QStringLiteral("primary_ipv4")).toString();
    cfg.secondaryIpv4 = obj.value(QStringLiteral("secondary_ipv4")).toString();
    cfg.primaryIpv6 = obj.value(QStringLiteral("primary_ipv6")).toString();
    cfg.secondaryIpv6 = obj.value(QStringLiteral("secondary_ipv6")).toString();
    return cfg;
}
