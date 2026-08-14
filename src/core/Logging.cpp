#include "core/Logging.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QtGlobal>

namespace {

constexpr qint64 kMaxLogSize = 512 * 1024;
QMutex g_mutex;

QString logDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/logs");
}

QString severityName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARN");
    case QtCriticalMsg:
        return QStringLiteral("ERROR");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }
    return QStringLiteral("?");
}

void messageHandler(QtMsgType type, const QMessageLogContext& /*context*/, const QString& message)
{
    QMutexLocker locker(&g_mutex);

    // Opened lazily on first message; kept open for the process lifetime.
    static QFile file;
    if (!file.isOpen()) {
        const QString path = logDir() + QLatin1Char('/') + QStringLiteral("dnsmanager.log");
        QDir().mkpath(QFileInfo(path).absolutePath());
        file.setFileName(path);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    if (!file.isOpen())
        return;

    if (file.size() > kMaxLogSize) { // rotate
        file.close();
        QFile::remove(file.fileName() + QStringLiteral(".old"));
        QFile::rename(file.fileName(), file.fileName() + QStringLiteral(".old"));
        file.open(QIODevice::WriteOnly | QIODevice::Append);
    }

    const QString line = QStringLiteral("%1 %2 %3\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs),
             severityName(type), message);
    file.write(line.toUtf8());
    file.flush();
}

} // namespace

QString Logging::defaultLogDir()
{
    return logDir();
}

QString Logging::defaultLogFilePath()
{
    return logDir() + QLatin1Char('/') + QStringLiteral("dnsmanager.log");
}

void Logging::installMessageHandler()
{
    qInstallMessageHandler(messageHandler);
}