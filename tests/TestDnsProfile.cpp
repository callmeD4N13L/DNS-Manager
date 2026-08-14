#include <QtTest>

#include "models/DnsProfile.hpp"

class TestDnsProfile : public QObject
{
    Q_OBJECT

private slots:
    void createProfile();
    void validity();
    void summary();
    void serversList();
    void jsonRoundTrip();
    void jsonWithoutIdAssignsNewId();
    void equality();
};

void TestDnsProfile::createProfile()
{
    const DnsProfile profile = DnsProfile::create(QStringLiteral("Cloudflare"));
    QVERIFY(!profile.id.isNull());
    QCOMPARE(profile.name, QStringLiteral("Cloudflare"));
    QVERIFY(!profile.isValid()); // no DNS servers yet
}

void TestDnsProfile::validity()
{
    DnsProfile valid = DnsProfile::create(QStringLiteral("Cloudflare"));
    valid.primaryIpv4 = QStringLiteral("1.1.1.1");
    valid.secondaryIpv4 = QStringLiteral("1.0.0.1");
    QVERIFY(valid.isValid());

    DnsProfile unnamed;
    unnamed.primaryIpv4 = QStringLiteral("1.1.1.1");
    QVERIFY(!unnamed.isValid()); // id is null

    DnsProfile blankName = DnsProfile::create(QStringLiteral("   "));
    blankName.primaryIpv4 = QStringLiteral("1.1.1.1");
    QVERIFY(!blankName.isValid()); // name is empty

    DnsProfile badIp = DnsProfile::create(QStringLiteral("Bad"));
    badIp.primaryIpv4 = QStringLiteral("999.1.1.1");
    QVERIFY(!badIp.isValid());

    DnsProfile noServers = DnsProfile::create(QStringLiteral("Empty"));
    QVERIFY(!noServers.isValid()); // must have at least one server
}

void TestDnsProfile::summary()
{
    DnsProfile profile = DnsProfile::create(QStringLiteral("Quad9"));
    profile.primaryIpv4 = QStringLiteral("9.9.9.9");
    profile.secondaryIpv4 = QStringLiteral("149.112.112.112");
    QCOMPARE(profile.summary(), QStringLiteral("9.9.9.9 / 149.112.112.112"));

    profile.primaryIpv6 = QStringLiteral("2620:fe::fe");
    QCOMPARE(profile.summary(),
             QStringLiteral("9.9.9.9 / 149.112.112.112 / 2620:fe::fe"));
}

void TestDnsProfile::serversList()
{
    DnsProfile profile = DnsProfile::create(QStringLiteral("Cloudflare"));
    profile.secondaryIpv4 = QStringLiteral("1.0.0.1");
    profile.primaryIpv4 = QStringLiteral("1.1.1.1");
    profile.primaryIpv6 = QStringLiteral("2606:4700:4700::1111");

    const QStringList expected = {
        QStringLiteral("1.1.1.1"),
        QStringLiteral("1.0.0.1"),
        QStringLiteral("2606:4700:4700::1111"),
    };
    QCOMPARE(profile.serversList(), expected);
}

void TestDnsProfile::jsonRoundTrip()
{
    DnsProfile profile = DnsProfile::create(QStringLiteral("AdGuard"));
    profile.provider = QStringLiteral("AdGuard Software Ltd.");
    profile.description = QStringLiteral("Ads & trackers");
    profile.primaryIpv4 = QStringLiteral("94.140.14.14");
    profile.secondaryIpv4 = QStringLiteral("94.140.15.15");
    profile.primaryIpv6 = QStringLiteral("2a10:50c0::ad1:ff");
    profile.isFavorite = true;

    const DnsProfile restored = DnsProfile::fromJson(profile.toJson());
    QCOMPARE(restored, profile);
    QCOMPARE(restored.id, profile.id);
    QCOMPARE(restored.id.toString(QUuid::WithoutBraces), restored.toJson().value("id").toString());
    QVERIFY(restored.isValid());
}

void TestDnsProfile::jsonWithoutIdAssignsNewId()
{
    QJsonObject obj;
    obj.insert(QStringLiteral("name"), QStringLiteral("NoId"));
    obj.insert(QStringLiteral("primary_ipv4"), QStringLiteral("8.8.8.8"));

    const DnsProfile profile = DnsProfile::fromJson(obj);
    QVERIFY(!profile.id.isNull());
    QCOMPARE(profile.name, QStringLiteral("NoId"));
    QVERIFY(profile.isValid());
}

void TestDnsProfile::equality()
{
    const DnsProfile a = DnsProfile::create(QStringLiteral("Cloudflare"));
    const DnsProfile b = DnsProfile::create(QStringLiteral("Cloudflare"));
    QVERIFY(a != b); // different ids

    DnsProfile copy = a;
    QCOMPARE(copy, a);
}

QTEST_GUILESS_MAIN(TestDnsProfile)
#include "TestDnsProfile.moc"
