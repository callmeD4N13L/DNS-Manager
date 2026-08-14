#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqml.h>

// Measures DNS query latency to a specific server by sending a raw DNS
// query over UDP (port 53) and timing the response. The blocking socket I/O
// runs on a worker thread (QtConcurrent) so the UI stays responsive.
class DnsLatencyTester : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit DnsLatencyTester(QObject* parent = nullptr);

    // Starts an asynchronous test of `serverAddress`. The result is delivered
    // through testFinished(). Multiple tests may be queued; each is independent.
    Q_INVOKABLE void test(const QString& serverAddress);

signals:
    // latencyMs is -1 and ok is false on failure (timeout, invalid address).
    void testFinished(const QString& serverAddress, double latencyMs, bool ok,
                      const QString& message);
};
