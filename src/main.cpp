#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include "core/CrashHandler.hpp"
#include "core/Logging.hpp"

#if defined(Q_OS_WIN)

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include "core/OperationResult.hpp"
#include "platform/WindowsDns.hpp"

// Runs as the elevated helper: applies a DNS request file that the GUI wrote,
// then writes "<requestPath>.result" with the outcome and exits.
// Expects: <exe> --apply-dns <requestPath>
int runElevatedApply(const QString& requestPath)
{
    const OperationResult applied = WindowsDns::applyDnsFromFile(requestPath);

    QJsonObject result;
    result.insert(QStringLiteral("success"), applied.success);
    result.insert(QStringLiteral("message"), applied.message);
    result.insert(QStringLiteral("error_code"), applied.errorCode);

    QFile out(requestPath + QStringLiteral(".result"));
    if (out.open(QIODevice::WriteOnly))
        out.write(QJsonDocument(result).toJson(QJsonDocument::Compact));

    return applied.success ? 0 : 1;
}

#endif // Q_OS_WIN

int main(int argc, char* argv[])
{
    Logging::installMessageHandler();
#if defined(Q_OS_WIN)
    CrashHandler::install();
#endif

    // QApplication (not QGuiApplication): the system tray context menu is a
    // QMenu, which lives in QtWidgets.
    QApplication app(argc, argv);
    QApplication::setApplicationDisplayName(QStringLiteral("DNS Manager"));
    QApplication::setApplicationName(QStringLiteral("DnsManager"));
    QApplication::setOrganizationName(QStringLiteral("DnsManager"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    qInfo("DnsManager %s starting (Qt %s)",
          qUtf8Printable(QCoreApplication::applicationVersion()), qVersion());

#if defined(Q_OS_WIN)
    // Headless, elevated entry point used by NetworkManager to apply DNS.
    const QStringList arguments = QCoreApplication::arguments();
    const int applyIndex = arguments.indexOf(QStringLiteral("--apply-dns"));
    if (applyIndex >= 0 && applyIndex + 1 < arguments.size())
        return runElevatedApply(arguments.at(applyIndex + 1));
#endif

    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app/app-icon-256.png")));

    // Base style for Qt Quick Controls. The application draws its own themed
    // components on top; Fusion is stable and style-independent.
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            qCritical("Failed to load the QML user interface");
            QMessageBox::critical(nullptr, QObject::tr("DNS Manager"),
                                  QObject::tr("The user interface could not be loaded.\n"
                                              "See the application log for details."));
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("DnsManager"), QStringLiteral("Main"));

    return app.exec();
}
