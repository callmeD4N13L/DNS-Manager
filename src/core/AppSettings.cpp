#include "core/AppSettings.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace {

const QString kThemeMode = QStringLiteral("appearance/themeMode");
const QString kStartWithWindows = QStringLiteral("behavior/startWithWindows");
const QString kStartMinimized = QStringLiteral("behavior/startMinimized");
const QString kMinimizeToTray = QStringLiteral("behavior/minimizeToTray");
const QString kFlushAfterApply = QStringLiteral("dns/flushCacheAfterApply");
const QString kConfirmBeforeApply = QStringLiteral("dns/confirmBeforeApply");

QString defaultThemeMode()
{
    return QStringLiteral("system");
}

} // namespace

AppSettings::AppSettings(QObject* parent)
    : QObject(parent)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               QStringLiteral("DnsManager"), QStringLiteral("DnsManager"), this))
{
    m_themeMode = m_settings->value(kThemeMode, defaultThemeMode()).toString();
    m_startWithWindows = m_settings->value(kStartWithWindows, false).toBool();
    m_startMinimized = m_settings->value(kStartMinimized, false).toBool();
    m_minimizeToTray = m_settings->value(kMinimizeToTray, true).toBool();
    m_flushCacheAfterApply = m_settings->value(kFlushAfterApply, true).toBool();
    m_confirmBeforeApply = m_settings->value(kConfirmBeforeApply, true).toBool();
}

QString AppSettings::themeMode() const
{
    return m_themeMode;
}

void AppSettings::setThemeMode(const QString& mode)
{
    const QString normalized = mode.toLower();
    if (normalized != QStringLiteral("system") && normalized != QStringLiteral("light")
        && normalized != QStringLiteral("dark"))
        return;
    if (normalized == m_themeMode)
        return;
    m_themeMode = normalized;
    m_settings->setValue(kThemeMode, m_themeMode);
    m_settings->sync();
    emit themeModeChanged();
}

bool AppSettings::startWithWindows() const
{
    return m_startWithWindows;
}

void AppSettings::setStartWithWindows(bool enabled)
{
    if (enabled == m_startWithWindows)
        return;
    m_startWithWindows = enabled;
    m_settings->setValue(kStartWithWindows, m_startWithWindows);
    m_settings->sync();
    applyStartupEntry();
    emit startWithWindowsChanged();
}

bool AppSettings::startMinimized() const
{
    return m_startMinimized;
}

void AppSettings::setStartMinimized(bool enabled)
{
    if (enabled == m_startMinimized)
        return;
    m_startMinimized = enabled;
    m_settings->setValue(kStartMinimized, m_startMinimized);
    m_settings->sync();
    applyStartupEntry(); // the Run command must stay in sync
    emit startMinimizedChanged();
}

bool AppSettings::minimizeToTray() const
{
    return m_minimizeToTray;
}

void AppSettings::setMinimizeToTray(bool enabled)
{
    if (enabled == m_minimizeToTray)
        return;
    m_minimizeToTray = enabled;
    m_settings->setValue(kMinimizeToTray, m_minimizeToTray);
    m_settings->sync();
    emit minimizeToTrayChanged();
}

bool AppSettings::flushCacheAfterApply() const
{
    return m_flushCacheAfterApply;
}

void AppSettings::setFlushCacheAfterApply(bool enabled)
{
    if (enabled == m_flushCacheAfterApply)
        return;
    m_flushCacheAfterApply = enabled;
    m_settings->setValue(kFlushAfterApply, m_flushCacheAfterApply);
    m_settings->sync();
    emit flushCacheAfterApplyChanged();
}

bool AppSettings::confirmBeforeApply() const
{
    return m_confirmBeforeApply;
}

void AppSettings::setConfirmBeforeApply(bool enabled)
{
    if (enabled == m_confirmBeforeApply)
        return;
    m_confirmBeforeApply = enabled;
    m_settings->setValue(kConfirmBeforeApply, m_confirmBeforeApply);
    m_settings->sync();
    emit confirmBeforeApplyChanged();
}

QString AppSettings::settingsFilePath() const
{
    return m_settings->fileName();
}

void AppSettings::resetToDefaults()
{
    m_themeMode = defaultThemeMode();
    m_startWithWindows = false;
    m_startMinimized = false;
    m_minimizeToTray = true;
    m_flushCacheAfterApply = true;
    m_confirmBeforeApply = true;

    m_settings->clear();
    m_settings->sync();
    applyStartupEntry();

    emit themeModeChanged();
    emit startWithWindowsChanged();
    emit startMinimizedChanged();
    emit minimizeToTrayChanged();
    emit flushCacheAfterApplyChanged();
    emit confirmBeforeApplyChanged();
}

void AppSettings::applyStartupEntry()
{
#if defined(Q_OS_WIN)
    if (QStandardPaths::isTestModeEnabled())
        return; // never touch the real registry from tests
    QSettings runKey(QSettings::NativeFormat, QSettings::UserScope, QStringLiteral("Microsoft"),
                     QStringLiteral("Windows\\CurrentVersion\\Run"));
    if (m_startWithWindows) {
        QString command = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        if (m_startMinimized)
            command += QStringLiteral(" --minimized");
        runKey.setValue(QStringLiteral("DnsManager"), command);
    } else {
        runKey.remove(QStringLiteral("DnsManager"));
    }
#else
    Q_UNUSED(m_startWithWindows)
#endif
}
