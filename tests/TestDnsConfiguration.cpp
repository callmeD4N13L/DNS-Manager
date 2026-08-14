#include <QtTest>

#include "models/DnsConfiguration.hpp"

class TestDnsConfiguration : public QObject
{
    Q_OBJECT

private slots:
    void ipv4Valid();
    void ipv4Invalid();
    void ipv6Valid();
    void ipv6Invalid();
    void configurationValidity();
    void hasAny_data();
    void hasAny();
    void serversListOrder();
    void jsonRoundTrip();
};

void TestDnsConfiguration::ipv4Valid()
{
    QVERIFY(Dns::isValidIpv4(QStringLiteral("1.1.1.1")));
    QVERIFY(Dns::isValidIpv4(QStringLiteral("8.8.8.8")));
    QVERIFY(Dns::isValidIpv4(QStringLiteral("0.0.0.0")));
    QVERIFY(Dns::isValidIpv4(QStringLiteral("255.255.255.255")));
    QVERIFY(Dns::isValidIpv4(QStringLiteral("9.9.9.9")));
    QVERIFY(Dns::isValidIpv4(QStringLiteral("94.140.14.14")));
}

void TestDnsConfiguration::ipv4Invalid()
{
    QVERIFY(!Dns::isValidIpv4(QString()));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1.1.1")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1.1.1.1.1")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("256.1.1.1")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1.256.1.1")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("999.1.1.1")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1.1.1.abc")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("-1.2.3.4")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1.2.3.4 ")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral(" 1.2.3.4")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1..3.4")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("127.1"))); // shorthand rejected
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("1.2.3")));
    QVERIFY(!Dns::isValidIpv4(QStringLiteral("not-an-ip")));
}

void TestDnsConfiguration::ipv6Valid()
{
    QVERIFY(Dns::isValidIpv6(QStringLiteral("::1")));
    QVERIFY(Dns::isValidIpv6(QStringLiteral("2001:4860:4860::8888")));
    QVERIFY(Dns::isValidIpv6(QStringLiteral("2606:4700:4700::1111")));
    QVERIFY(Dns::isValidIpv6(QStringLiteral("2a10:50c0::ad1:ff")));
    QVERIFY(Dns::isValidIpv6(QStringLiteral("fe80::")));
}

void TestDnsConfiguration::ipv6Invalid()
{
    QVERIFY(!Dns::isValidIpv6(QString()));
    QVERIFY(!Dns::isValidIpv6(QStringLiteral("not-an-ip")));
    QVERIFY(!Dns::isValidIpv6(QStringLiteral("1.2.3.4")));
    QVERIFY(!Dns::isValidIpv6(QStringLiteral("gggg::1")));
    QVERIFY(!Dns::isValidIpv6(QStringLiteral("1::2::3")));
}

void TestDnsConfiguration::configurationValidity()
{
    DnsConfiguration empty;
    QVERIFY(empty.isValid());
    QVERIFY(!empty.hasAny());

    DnsConfiguration good;
    good.primaryIpv4 = QStringLiteral("1.1.1.1");
    QVERIFY(good.isValid());
    QVERIFY(good.hasAny());

    DnsConfiguration bad;
    bad.primaryIpv4 = QStringLiteral("999.1.1.1");
    QVERIFY(!bad.isValid());

    DnsConfiguration badV6;
    badV6.primaryIpv6 = QStringLiteral("gggg::1");
    QVERIFY(!badV6.isValid());
}

void TestDnsConfiguration::hasAny_data()
{
    QTest::addColumn<QString>("p4");
    QTest::addColumn<QString>("s4");
    QTest::addColumn<QString>("p6");
    QTest::addColumn<QString>("s6");
    QTest::addColumn<bool>("expected");

    QTest::newRow("empty") << "" << "" << "" << "" << false;
    QTest::newRow("ipv4 primary") << "1.1.1.1" << "" << "" << "" << true;
    QTest::newRow("ipv4 secondary") << "" << "8.8.4.4" << "" << "" << true;
    QTest::newRow("ipv6 primary") << "" << "" << "::1" << "" << true;
    QTest::newRow("ipv6 secondary") << "" << "" << "" << "::1" << true;
    QTest::newRow("all") << "1.1.1.1" << "1.0.0.1" << "2606:4700:4700::1111"
                         << "2606:4700:4700::1001" << true;
}

void TestDnsConfiguration::hasAny()
{
    QFETCH(QString, p4);
    QFETCH(QString, s4);
    QFETCH(QString, p6);
    QFETCH(QString, s6);
    QFETCH(bool, expected);

    const DnsConfiguration cfg { p4, s4, p6, s6 };
    QCOMPARE(cfg.hasAny(), expected);
}

void TestDnsConfiguration::serversListOrder()
{
    DnsConfiguration cfg;
    cfg.secondaryIpv6 = QStringLiteral("2606:4700:4700::1001");
    cfg.primaryIpv4 = QStringLiteral("1.1.1.1");
    cfg.secondaryIpv4 = QStringLiteral("1.0.0.1");
    cfg.primaryIpv6 = QStringLiteral("2606:4700:4700::1111");

    const QStringList expected = {
        QStringLiteral("1.1.1.1"),
        QStringLiteral("1.0.0.1"),
        QStringLiteral("2606:4700:4700::1111"),
        QStringLiteral("2606:4700:4700::1001"),
    };
    QCOMPARE(cfg.serversList(), expected);

    DnsConfiguration partial;
    partial.primaryIpv4 = QStringLiteral("9.9.9.9");
    QCOMPARE(partial.serversList(), QStringList { QStringLiteral("9.9.9.9") });
}

void TestDnsConfiguration::jsonRoundTrip()
{
    const DnsConfiguration original { QStringLiteral("1.1.1.1"), QStringLiteral("1.0.0.1"),
                                      QStringLiteral("2606:4700:4700::1111"),
                                      QStringLiteral("2606:4700:4700::1001") };
    const DnsConfiguration restored = DnsConfiguration::fromJson(original.toJson());
    QCOMPARE(restored, original);

    const DnsConfiguration empty = DnsConfiguration::fromJson(DnsConfiguration().toJson());
    QCOMPARE(empty, DnsConfiguration());
}

QTEST_GUILESS_MAIN(TestDnsConfiguration)
#include "TestDnsConfiguration.moc"
