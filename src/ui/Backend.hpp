#pragma once

#include <QJSEngine>
#include <QMap>
#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QtQml/qqml.h>

class AdapterListModel;
class AppSettings;
class DnsLatencyTester;
class NetworkManager;
class ProfileListModel;
class ProfileManager;
class QQuickWindow;
class TrayController;

// Application-wide bridge between C++ and QML.
// Registered as a QML singleton ("Backend") inside the DnsManager module.
class Backend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString platform READ platform CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(QString aboutText READ aboutText CONSTANT)
    // Connectivity state of the selected adapter (updated on refresh/selection).
    Q_PROPERTY(bool currentAdapterConnected READ currentAdapterConnected NOTIFY connectionChanged)
    Q_PROPERTY(ProfileListModel* profiles READ profiles CONSTANT)
    Q_PROPERTY(AdapterListModel* adapters READ adapters NOTIFY adaptersChanged)
    Q_PROPERTY(QString selectedAdapterId READ selectedAdapterId NOTIFY selectedAdapterChanged)
    // Live DNS state of the selected adapter. Empty strings mean "not set".
    Q_PROPERTY(QString currentDnsAdapterName READ currentDnsAdapterName NOTIFY currentDnsChanged)
    Q_PROPERTY(QString currentDnsPrimaryIpv4 READ currentDnsPrimaryIpv4 NOTIFY currentDnsChanged)
    Q_PROPERTY(QString currentDnsSecondaryIpv4 READ currentDnsSecondaryIpv4 NOTIFY currentDnsChanged)
    Q_PROPERTY(QString currentDnsPrimaryIpv6 READ currentDnsPrimaryIpv6 NOTIFY currentDnsChanged)
    Q_PROPERTY(QString currentDnsSecondaryIpv6 READ currentDnsSecondaryIpv6 NOTIFY currentDnsChanged)
    Q_PROPERTY(bool currentDnsIsDhcp READ currentDnsIsDhcp NOTIFY currentDnsChanged)
    Q_PROPERTY(bool currentDnsAvailable READ currentDnsAvailable NOTIFY currentDnsChanged)
    // Profile that was last applied / will be applied by Dashboard's Apply.
    Q_PROPERTY(QString activeProfileId READ activeProfileId NOTIFY activeProfileChanged)
    Q_PROPERTY(QString activeProfileName READ activeProfileName NOTIFY activeProfileChanged)
    // Best DNS latency (ms) of the selected adapter after testCurrentDns(); -1 = no success.
    Q_PROPERTY(int dnsLatencyMs READ dnsLatencyMs NOTIFY dnsTestFinished)
    // Preferences; all theme/startup/tray toggles live on this object.
    Q_PROPERTY(AppSettings* settings READ settings CONSTANT)
    // Effective dark theme (follows settings + system color scheme).
    Q_PROPERTY(bool darkMode READ darkMode NOTIFY darkModeChanged)
    // True once a quit was requested through the tray (Main.qml then lets the
    // window actually close instead of hiding to tray).
    Q_PROPERTY(bool isQuitting READ isQuitting NOTIFY quittingChanged)
    Q_PROPERTY(bool startHidden READ startHidden CONSTANT)
    Q_PROPERTY(QString profilesFilePath READ profilesFilePath CONSTANT)
    Q_PROPERTY(QString settingsFilePath READ settingsFilePath CONSTANT)

public:
    explicit Backend(QObject* parent = nullptr);

    static Backend* create(QQmlEngine* engine, QJSEngine* jsEngine);

    QString version() const;
    QString platform() const;
    QString qtVersion() const;
    QString aboutText() const;
    bool currentAdapterConnected() const;
    ProfileListModel* profiles() const;
    AdapterListModel* adapters() const;
    QString selectedAdapterId() const;
    QString currentDnsAdapterName() const;
    QString currentDnsPrimaryIpv4() const;
    QString currentDnsSecondaryIpv4() const;
    QString currentDnsPrimaryIpv6() const;
    QString currentDnsSecondaryIpv6() const;
    bool currentDnsIsDhcp() const;
    bool currentDnsAvailable() const;
    QString activeProfileId() const;
    QString activeProfileName() const;

    // Applies a saved profile to the selected adapter (elevates as needed).
    Q_INVOKABLE void applyProfile(const QString& id);
    Q_INVOKABLE void applyActive();
    Q_INVOKABLE void resetDns();

    // Flushes the system DNS resolver cache.
    Q_INVOKABLE void flushDnsCache();

    // Measures latency to every DNS server of the selected adapter.
    Q_INVOKABLE void testCurrentDns();

    int dnsLatencyMs() const;
    AppSettings* settings() const;
    bool darkMode() const;
    bool isQuitting() const;
    bool startHidden() const;
    QString profilesFilePath() const;
    QString settingsFilePath() const;

    // Profile import/export/reset (paths may be file: URLs or plain paths).
    Q_INVOKABLE bool exportProfiles(const QString& location);
    Q_INVOKABLE bool importProfiles(const QString& location);
    Q_INVOKABLE void resetConfiguration();

    // Window / tray control (called from QML and the tray menu).
    Q_INVOKABLE void setMainWindow(QObject* window);
    Q_INVOKABLE void hideToTray();
    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void requestQuit();

    // Re-detects adapters and picks an active one if none is selected.
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void selectAdapter(const QString& id);

    // Shows a transient message in the UI (toast). The backend only emits the
    // request; the QML layer decides how to render it.
    Q_INVOKABLE void notify(const QString& text, const QString& kind = QStringLiteral("info"));

signals:
    void adaptersChanged();
    void selectedAdapterChanged();
    void currentDnsChanged();
    void connectionChanged();
    void activeProfileChanged();
    void dnsTestFinished();
    void darkModeChanged();
    void quittingChanged();
    void notificationRequested(const QString& text, const QString& kind);

private:
    void startNextLatencyTest();
    void onLatencyTestFinished(const QString& server, double latencyMs, bool ok,
                               const QString& message);
    void finalizeLatencyTest();
    void emitDarkModeChanged();
    static QString toLocalPath(const QString& location);

    ProfileManager* m_profileManager = nullptr;
    ProfileListModel* m_profiles = nullptr;
    NetworkManager* m_networkManager = nullptr;
    AdapterListModel* m_adapters = nullptr;
    DnsLatencyTester* m_latencyTester = nullptr;
    AppSettings* m_settings = nullptr;
    TrayController* m_tray = nullptr;
    QQuickWindow* m_window = nullptr;
    QString m_activeProfileId;
    int m_dnsLatencyMs = -1;
    bool m_isQuitting = false;
    QStringList m_testQueue;
    QStringList m_testMessages;
    QMap<QString, double> m_testLatencies;
};
