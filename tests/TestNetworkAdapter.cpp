#include <QtTest>

#include "models/NetworkAdapter.hpp"

class TestNetworkAdapter : public QObject
{
    Q_OBJECT

private slots:
    void typeToString();
    void defaultConstructed();
    void equality();
};

void TestNetworkAdapter::typeToString()
{
    QCOMPARE(NetworkAdapter::typeToString(NetworkAdapter::Type::Ethernet), QStringLiteral("Ethernet"));
    QCOMPARE(NetworkAdapter::typeToString(NetworkAdapter::Type::Wifi), QStringLiteral("Wi-Fi"));
    QCOMPARE(NetworkAdapter::typeToString(NetworkAdapter::Type::Virtual), QStringLiteral("Virtual"));
    QCOMPARE(NetworkAdapter::typeToString(NetworkAdapter::Type::Vpn), QStringLiteral("VPN"));
    QCOMPARE(NetworkAdapter::typeToString(NetworkAdapter::Type::Loopback), QStringLiteral("Loopback"));
    QCOMPARE(NetworkAdapter::typeToString(NetworkAdapter::Type::Other), QStringLiteral("Other"));
}

void TestNetworkAdapter::defaultConstructed()
{
    NetworkAdapter adapter;
    QVERIFY(adapter.id.isNull());
    QVERIFY(adapter.name.isEmpty());
    QCOMPARE(adapter.type, NetworkAdapter::Type::Other);
    QCOMPARE(adapter.enabled, false);
    QCOMPARE(adapter.connected, false);
    QCOMPARE(adapter.dhcpEnabled, false);
}

void TestNetworkAdapter::equality()
{
    NetworkAdapter a;
    a.id = QUuid::createUuid();
    a.name = QStringLiteral("Ethernet 2");
    a.friendlyName = QStringLiteral("Realtek Ethernet");
    a.type = NetworkAdapter::Type::Ethernet;
    a.enabled = true;
    a.connected = true;
    a.dhcpEnabled = true;
    a.ipv4Addresses = { QStringLiteral("192.168.1.10") };
    a.dnsServers = { QStringLiteral("1.1.1.1") };

    NetworkAdapter b = a;
    QCOMPARE(a, b);

    b.connected = false;
    QVERIFY(a != b);
}

QTEST_GUILESS_MAIN(TestNetworkAdapter)
#include "TestNetworkAdapter.moc"
