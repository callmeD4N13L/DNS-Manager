#include <QtTest>

#include <QFile>
#include <QFileInfo>

#include "core/Logging.hpp"

class TestLogging : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void writesMessagesToLog();
    void rotatesAfterSizeLimit();
};

void TestLogging::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestLogging::cleanupTestCase()
{
    qInstallMessageHandler(nullptr);
    QFile::remove(Logging::defaultLogFilePath());
    QFile::remove(Logging::defaultLogFilePath() + QStringLiteral(".old"));
}

void TestLogging::writesMessagesToLog()
{
    const QString logPath = Logging::defaultLogFilePath();
    QVERIFY(!logPath.isEmpty());
    QVERIFY(!Logging::defaultLogDir().isEmpty());
    QFile::remove(logPath);

    Logging::installMessageHandler();
    qWarning("LoggingTestMarker 12345");
    qInstallMessageHandler(nullptr);

    QVERIFY(QFile::exists(logPath));
    QFile log(logPath);
    QVERIFY(log.open(QIODevice::ReadOnly));
    const QByteArray content = log.readAll();
    log.close();

    QVERIFY2(content.contains("LoggingTestMarker 12345"),
             "log file must contain the emitted message");
    QVERIFY(content.contains("WARN")); // severity tag present
    QVERIFY(content.contains("202")); // ISO timestamp year prefix
}

void TestLogging::rotatesAfterSizeLimit()
{
    const QString logPath = Logging::defaultLogFilePath();
    const QString oldPath = logPath + QStringLiteral(".old");
    QFile::remove(logPath);
    QFile::remove(oldPath);

    Logging::installMessageHandler();

    // ~1 KB per message; the file rotates once it exceeds ~512 KB.
    const QString padding(1000, QLatin1Char('x'));
    for (int i = 0; i < 700; ++i)
        qWarning("%05d %s", i, qPrintable(padding));

    qInstallMessageHandler(nullptr);

    QVERIFY2(QFile::exists(oldPath), "log must rotate and keep the previous file as .old");
    QVERIFY2(QFile::exists(logPath), "a fresh log must exist after rotation");

    QFile rotated(oldPath);
    QVERIFY(rotated.open(QIODevice::ReadOnly));
    QVERIFY(rotated.size() > 100 * 1024); // old file kept a substantial chunk
    rotated.close();
}

QTEST_GUILESS_MAIN(TestLogging)
#include "TestLogging.moc"
