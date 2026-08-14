#include "models/DnsProfile.hpp"

#include <QStringList>

#include "models/DnsConfiguration.hpp"

DnsProfile DnsProfile::create(const QString& name)
{
    DnsProfile profile;
    profile.id = QUuid::createUuid();
    profile.name = name;
    return profile;
}

bool DnsProfile::isValid() const
{
    if (name.trimmed().isEmpty())
        return false;
    if (id.isNull())
        return false;

    const DnsConfiguration dns {
        primaryIpv4, secondaryIpv4, primaryIpv6, secondaryIpv6
    };
    return dns.isValid() && dns.hasAny();
}

QString DnsProfile::summary() const
{
    QStringList parts;
    if (!primaryIpv4.isEmpty())
        parts << primaryIpv4;
    if (!secondaryIpv4.isEmpty())
        parts << secondaryIpv4;
    if (!primaryIpv6.isEmpty())
        parts << primaryIpv6;
    if (!secondaryIpv6.isEmpty())
        parts << secondaryIpv6;
    return parts.join(QStringLiteral(" / "));
}

QStringList DnsProfile::serversList() const
{
    const DnsConfiguration dns { primaryIpv4, secondaryIpv4, primaryIpv6, secondaryIpv6 };
    return dns.serversList();
}

QJsonObject DnsProfile::toJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), id.toString(QUuid::WithoutBraces));
    obj.insert(QStringLiteral("name"), name);
    obj.insert(QStringLiteral("provider"), provider);
    obj.insert(QStringLiteral("description"), description);
    obj.insert(QStringLiteral("primary_ipv4"), primaryIpv4);
    obj.insert(QStringLiteral("secondary_ipv4"), secondaryIpv4);
    obj.insert(QStringLiteral("primary_ipv6"), primaryIpv6);
    obj.insert(QStringLiteral("secondary_ipv6"), secondaryIpv6);
    obj.insert(QStringLiteral("favorite"), isFavorite);
    return obj;
}

DnsProfile DnsProfile::fromJson(const QJsonObject& obj)
{
    DnsProfile profile;
    const QString id = obj.value(QStringLiteral("id")).toString();
    if (!id.isEmpty())
        profile.id = QUuid(id);
    if (profile.id.isNull())
        profile.id = QUuid::createUuid();
    profile.name = obj.value(QStringLiteral("name")).toString();
    profile.provider = obj.value(QStringLiteral("provider")).toString();
    profile.description = obj.value(QStringLiteral("description")).toString();
    profile.primaryIpv4 = obj.value(QStringLiteral("primary_ipv4")).toString();
    profile.secondaryIpv4 = obj.value(QStringLiteral("secondary_ipv4")).toString();
    profile.primaryIpv6 = obj.value(QStringLiteral("primary_ipv6")).toString();
    profile.secondaryIpv6 = obj.value(QStringLiteral("secondary_ipv6")).toString();
    profile.isFavorite = obj.value(QStringLiteral("favorite")).toBool();
    return profile;
}
