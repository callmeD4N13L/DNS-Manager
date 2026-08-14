#pragma once

#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QUuid>

// A saved, user-defined DNS configuration.
class DnsProfile
{
public:
    QUuid id;
    QString name;
    QString provider;
    QString description;
    QString primaryIpv4;
    QString secondaryIpv4;
    QString primaryIpv6;
    QString secondaryIpv6;
    bool isFavorite = false;

    static DnsProfile create(const QString& name);

    bool isValid() const;
    QString summary() const;
    QStringList serversList() const;
    bool operator==(const DnsProfile& other) const = default;

    QJsonObject toJson() const;
    static DnsProfile fromJson(const QJsonObject& obj);
};

Q_DECLARE_METATYPE(DnsProfile)