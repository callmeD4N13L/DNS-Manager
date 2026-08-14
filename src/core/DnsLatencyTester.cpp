#include "core/DnsLatencyTester.hpp"

#include <QDataStream>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QHostAddress>
#include <QObject>
#include <QRandomGenerator>
#include <QUdpSocket>
#include <QtConcurrent/QtConcurrent>

namespace {

constexpr int kTimeoutMs = 3000;
constexpr quint16 kDnsPort = 53;

struct LatencyResult
{
    QString server;
    double latencyMs = -1;
    bool ok = false;
    QString message;
};

// Standard DNS query: header (RD flag) + one A-record question.
QByteArray buildDnsQuery(quint16 id, const QString& name)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << id << quint16(0x0100) << quint16(1) << quint16(0) << quint16(0) << quint16(0);

    const QStringList labels = name.split(QLatin1Char('.'));
    for (const QString& label : labels) {
        const QByteArray bytes = label.toLatin1();
        stream << static_cast<quint8>(bytes.size());
        stream.writeRawData(bytes.constData(), bytes.size());
    }
    stream << static_cast<quint8>(0);
    stream << quint16(1) << quint16(1); // QTYPE A, QCLASS IN
    return packet;
}

LatencyResult measureLatency(const QString& server)
{
    const QHostAddress address(server);
    if (address.isNull())
        return { server, -1, false, QObject::tr("Invalid server address") };

    // Random subdomain keeps the answer from being served from a cache.
    const QString probeName = QStringLiteral("dnsmgr-latency-%1.invalid").arg(
        QRandomGenerator::global()->bounded(0x7fffffffu), 8, 16, QLatin1Char('0'));
    const quint16 queryId = static_cast<quint16>(QRandomGenerator::global()->bounded(0x10000));

    QUdpSocket socket;
    if (socket.writeDatagram(buildDnsQuery(queryId, probeName), address, kDnsPort) == -1)
        return { server, -1, false, QObject::tr("Could not send query") };

    QElapsedTimer timer;
    timer.start();

    QHostAddress from;
    quint16 fromPort = 0;
    while (timer.elapsed() < kTimeoutMs) {
        if (!socket.waitForReadyRead(kTimeoutMs - static_cast<int>(timer.elapsed())))
            break;
        while (socket.hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(socket.pendingDatagramSize());
            socket.readDatagram(datagram.data(), datagram.size(), &from, &fromPort);
            if (from != address || fromPort != kDnsPort || datagram.size() < 2)
                continue;
            const quint16 responseId = (static_cast<quint16>(static_cast<quint8>(datagram[0])) << 8)
                | static_cast<quint8>(datagram[1]);
            if (responseId == queryId)
                return { server, static_cast<double>(timer.elapsed()), true, QString() };
        }
    }

    return { server, -1, false,
             QObject::tr("No response within %1 ms").arg(kTimeoutMs) };
}

} // namespace

DnsLatencyTester::DnsLatencyTester(QObject* parent)
    : QObject(parent)
{
}

void DnsLatencyTester::test(const QString& serverAddress)
{
    auto* watcher = new QFutureWatcher<LatencyResult>(this);
    connect(watcher, &QFutureWatcher<LatencyResult>::finished, this,
            [this, watcher, serverAddress]() {
                const LatencyResult result = watcher->result();
                emit testFinished(result.server, result.latencyMs, result.ok, result.message);
                watcher->deleteLater();
            });
    watcher->setFuture(
        QtConcurrent::run([serverAddress]() { return measureLatency(serverAddress); }));
}
