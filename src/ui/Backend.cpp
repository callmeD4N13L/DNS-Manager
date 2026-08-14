#include "ui/Backend.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include <QUrl>
#include <QtGlobal>
#include <algorithm>

#include "core/AppSettings.hpp"
#include "core/DnsLatencyTester.hpp"
#include "core/NetworkManager.hpp"
#include "core/ProfileManager.hpp"
#include "core/ProfileStorage.hpp"
#include "models/DnsProfile.hpp"
#include "ui/AdapterListModel.hpp"
#include "ui/ProfileListModel.hpp"
#include "ui/TrayController.hpp"

Backend* Backend::create(QQmlEngine*, QJSEngine*)
{
    return new Backend;
}

Backend::Backend(QObject* parent)
    : QObject(parent)
    , m_profileManager(new ProfileManager(this))
    , m_profiles(new ProfileListModel(m_profileManager, this))
    , m_networkManager(new NetworkManager(this))
    , m_adapters(new AdapterListModel(this))
    , m_latencyTester(new DnsLatencyTester(this))
    , m_settings(new AppSettings(this))
    , m_tray(new TrayController(this, this))
{
    m_profileManager->load();
    connect(m_profileManager, &ProfileManager::storageError,
            this, [this](const QString& message) {
                notify(tr("Could not save profiles: %1").arg(message), "error");
            });
    m_networkManager->refresh();
    connect(m_networkManager, &NetworkManager::currentDnsChanged,
            this, &Backend::currentDnsChanged);
    connect(m_networkManager, &NetworkManager::currentDnsChanged,
            this, &Backend::connectionChanged);
    connect(m_latencyTester, &DnsLatencyTester::testFinished,
            this, &Backend::onLatencyTestFinished);
    connect(m_settings, &AppSettings::themeModeChanged,
            this, &Backend::emitDarkModeChanged);
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, &Backend::emitDarkModeChanged);
}

QString Backend::version() const
{
    return QCoreApplication::applicationVersion();
}

QString Backend::platform() const
{
#if defined(Q_OS_WIN)
    return QStringLiteral("Windows");
#else
    return QStringLiteral("Unknown");
#endif
}

QString Backend::qtVersion() const
{
    return QString::fromUtf8(qVersion());
}

QString Backend::aboutText() const
{
    return tr("DNS Manager %1 — a modern DNS manager for Windows 10/11.\n"
              "Built with Qt %2 (C++20 + QML).")
        .arg(version(), qtVersion());
}

bool Backend::currentAdapterConnected() const
{
    const NetworkAdapter* adapter = m_networkManager->selectedAdapter();
    return adapter != nullptr && adapter->enabled && adapter->connected;
}

ProfileListModel* Backend::profiles() const
{
    return m_profiles;
}

AdapterListModel* Backend::adapters() const
{
    return m_adapters;
}

QString Backend::selectedAdapterId() const
{
    return m_networkManager->selectedAdapterId();
}

void Backend::refresh()
{
    const OperationResult result = m_networkManager->refresh();

    QList<NetworkAdapter> adapters;
    adapters.reserve(static_cast<int>(m_networkManager->adapters().size()));
    for (const NetworkAdapter& adapter : m_networkManager->adapters())
        adapters.append(adapter);
    m_adapters->setAdapters(adapters);
    m_adapters->setSelectedId(m_networkManager->selectedAdapterId());

    emit adaptersChanged();
    emit selectedAdapterChanged();
    emit connectionChanged();

    if (!result.success) {
        notify(tr("Failed to detect network adapters: %1").arg(result.message), "error");
        return;
    }
    if (m_networkManager->adapters().empty())
        notify(tr("No network adapters were detected"), "warning");
}

void Backend::selectAdapter(const QString& id)
{
    m_networkManager->setSelectedAdapter(id);
    m_adapters->setSelectedId(id);
    emit selectedAdapterChanged();
    emit connectionChanged();
}

QString Backend::currentDnsAdapterName() const
{
    const NetworkAdapter* adapter = m_networkManager->selectedAdapter();
    return adapter != nullptr ? adapter->friendlyName : QString();
}

QString Backend::currentDnsPrimaryIpv4() const
{
    return m_networkManager->currentDns().configuration.primaryIpv4;
}

QString Backend::currentDnsSecondaryIpv4() const
{
    return m_networkManager->currentDns().configuration.secondaryIpv4;
}

QString Backend::currentDnsPrimaryIpv6() const
{
    return m_networkManager->currentDns().configuration.primaryIpv6;
}

QString Backend::currentDnsSecondaryIpv6() const
{
    return m_networkManager->currentDns().configuration.secondaryIpv6;
}

bool Backend::currentDnsIsDhcp() const
{
    return m_networkManager->currentDns().isDhcp;
}

bool Backend::currentDnsAvailable() const
{
    return m_networkManager->selectedAdapter() != nullptr
        && m_networkManager->currentDns().success;
}

QString Backend::activeProfileId() const
{
    return m_activeProfileId;
}

QString Backend::activeProfileName() const
{
    const DnsProfile* profile = m_profileManager->profileById(m_activeProfileId);
    return profile != nullptr ? profile->name : QString();
}

void Backend::applyProfile(const QString& id)
{
    const DnsProfile* profile = m_profileManager->profileById(id);
    if (profile == nullptr) {
        notify(tr("Profile not found"), "error");
        return;
    }

    m_activeProfileId = id;
    emit activeProfileChanged();

    const QString adapterId = m_networkManager->selectedAdapterId();
    if (adapterId.isEmpty()) {
        notify(tr("No network adapter selected"), "error");
        return;
    }

    const OperationResult op = m_networkManager->applyDns(adapterId, profile->serversList());
    if (op.success) {
        if (m_settings->flushCacheAfterApply())
            m_networkManager->flushDnsCache();
        notify(tr("Applied “%1” to %2").arg(profile->name, currentDnsAdapterName()), "success");
    } else {
        notify(tr("Could not apply “%1”: %2").arg(profile->name, op.message), "error");
    }
}

void Backend::applyActive()
{
    if (m_activeProfileId.isEmpty()) {
        notify(tr("Select a profile to apply first"), "info");
        return;
    }
    applyProfile(m_activeProfileId);
}

void Backend::resetDns()
{
    const QString adapterId = m_networkManager->selectedAdapterId();
    if (adapterId.isEmpty()) {
        notify(tr("No network adapter selected"), "error");
        return;
    }

    // Avoid a pointless UAC prompt when DNS is already automatic.
    const DnsReadResult& current = m_networkManager->currentDns();
    if (current.success && current.isDhcp && !current.configuration.hasAny()) {
        notify(tr("DNS is already set to automatic (DHCP)"), "info");
        return;
    }

    const OperationResult op = m_networkManager->resetDns(adapterId);
    if (op.success) {
        if (m_settings->flushCacheAfterApply())
            m_networkManager->flushDnsCache();
        notify(tr("DNS reset to DHCP on %1").arg(currentDnsAdapterName()), "success");
    } else {
        notify(tr("Could not reset DNS: %1").arg(op.message), "error");
    }
}

void Backend::flushDnsCache()
{
    const OperationResult op = m_networkManager->flushDnsCache();
    if (op.success)
        notify(tr("DNS resolver cache flushed"), "success");
    else
        notify(tr("Could not flush the DNS cache: %1").arg(op.message), "error");
}

int Backend::dnsLatencyMs() const
{
    return m_dnsLatencyMs;
}

void Backend::testCurrentDns()
{
    const NetworkAdapter* adapter = m_networkManager->selectedAdapter();
    if (adapter == nullptr) {
        notify(tr("No network adapter selected"), "error");
        m_dnsLatencyMs = -1;
        emit dnsTestFinished();
        return;
    }
    if (adapter->dnsServers.isEmpty()) {
        notify(tr("No DNS servers configured on this adapter"), "info");
        m_dnsLatencyMs = -1;
        emit dnsTestFinished();
        return;
    }

    m_testQueue = adapter->dnsServers;
    m_testMessages.clear();
    m_testLatencies.clear();
    m_dnsLatencyMs = -1;
    notify(tr("Testing %1 DNS server(s)...").arg(m_testQueue.size()), "info");
    startNextLatencyTest();
}

void Backend::startNextLatencyTest()
{
    if (m_testQueue.isEmpty()) {
        finalizeLatencyTest();
        return;
    }
    m_latencyTester->test(m_testQueue.takeFirst());
}

void Backend::onLatencyTestFinished(const QString& server, double latencyMs, bool ok,
                                    const QString& message)
{
    if (ok)
        m_testLatencies.insert(server, latencyMs);
    else
        m_testMessages.append(tr("%1: %2").arg(server, message));
    startNextLatencyTest();
}

void Backend::finalizeLatencyTest()
{
    if (m_testLatencies.isEmpty()) {
        m_dnsLatencyMs = -1;
        notify(tr("All DNS servers timed out%1")
                   .arg(m_testMessages.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(m_testMessages.join(QStringLiteral("; ")))),
               "error");
        emit dnsTestFinished();
        return;
    }

    double best = m_testLatencies.begin().value();
    QString bestServer = m_testLatencies.begin().key();
    for (auto it = std::next(m_testLatencies.begin()); it != m_testLatencies.end(); ++it) {
        if (it.value() < best) {
            best = it.value();
            bestServer = it.key();
        }
    }

    m_dnsLatencyMs = qRound(best);
    notify(tr("Fastest DNS latency: %1 ms via %2").arg(m_dnsLatencyMs).arg(bestServer), "success");
    emit dnsTestFinished();
}

AppSettings* Backend::settings() const
{
    return m_settings;
}

bool Backend::darkMode() const
{
    const QString mode = m_settings->themeMode();
    if (mode == QLatin1String("light"))
        return false;
    if (mode == QLatin1String("dark"))
        return true;
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

bool Backend::isQuitting() const
{
    return m_isQuitting;
}

bool Backend::startHidden() const
{
    return m_settings->startMinimized()
        || QCoreApplication::arguments().contains(QStringLiteral("--minimized"));
}

QString Backend::profilesFilePath() const
{
    return ProfileStorage::filePath();
}

QString Backend::settingsFilePath() const
{
    return m_settings->settingsFilePath();
}

QString Backend::toLocalPath(const QString& location)
{
    const QUrl url(location);
    return url.isLocalFile() ? url.toLocalFile() : location;
}

bool Backend::exportProfiles(const QString& location)
{
    const QString path = toLocalPath(location);
    if (m_profileManager->exportToFile(path)) {
        notify(tr("Exported %1 profile(s)").arg(m_profiles->rowCount()), "success");
        return true;
    }
    notify(tr("Could not export profiles"), "error");
    return false;
}

bool Backend::importProfiles(const QString& location)
{
    const QString path = toLocalPath(location);
    if (m_profileManager->importFromFile(path)) {
        notify(tr("Profiles imported"), "success");
        return true;
    }
    notify(tr("Could not import profiles: the file may be missing or invalid"), "error");
    return false;
}

void Backend::resetConfiguration()
{
    m_settings->resetToDefaults();
    m_profileManager->resetToDefaults();
    emit darkModeChanged();
    notify(tr("Configuration reset to defaults"), "success");
}

void Backend::setMainWindow(QObject* window)
{
    m_window = qobject_cast<QQuickWindow*>(window);
    if (m_tray != nullptr)
        m_tray->setWindow(m_window);
}

void Backend::hideToTray()
{
    if (m_tray != nullptr && m_tray->isAvailable())
        m_tray->hideMainWindow();
    else if (m_window != nullptr)
        m_window->hide();
}

void Backend::showWindow()
{
    if (m_tray != nullptr && m_tray->isAvailable())
        m_tray->showMainWindow();
    else if (m_window != nullptr) {
        m_window->show();
        m_window->requestActivate();
    }
}

void Backend::requestQuit()
{
    m_isQuitting = true;
    emit quittingChanged();
    QCoreApplication::quit();
}

void Backend::emitDarkModeChanged()
{
    emit darkModeChanged();
}

void Backend::notify(const QString& text, const QString& kind)
{
    emit notificationRequested(text, kind);
}
