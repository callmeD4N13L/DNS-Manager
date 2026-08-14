#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class QSettings;

// Application preferences persisted in AppData (QSettings, INI format).
// Every setter writes through immediately and emits the matching signal.
// startWithWindows also maintains the HKCU Run registry entry (no admin).
class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(bool startWithWindows READ startWithWindows WRITE setStartWithWindows
                   NOTIFY startWithWindowsChanged)
    Q_PROPERTY(bool startMinimized READ startMinimized WRITE setStartMinimized
                   NOTIFY startMinimizedChanged)
    Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray
                   NOTIFY minimizeToTrayChanged)
    Q_PROPERTY(bool flushCacheAfterApply READ flushCacheAfterApply WRITE setFlushCacheAfterApply
                   NOTIFY flushCacheAfterApplyChanged)
    Q_PROPERTY(bool confirmBeforeApply READ confirmBeforeApply WRITE setConfirmBeforeApply
                   NOTIFY confirmBeforeApplyChanged)

public:
    explicit AppSettings(QObject* parent = nullptr);

    QString themeMode() const; // "system", "light" or "dark"
    void setThemeMode(const QString& mode);

    bool startWithWindows() const;
    void setStartWithWindows(bool enabled);

    bool startMinimized() const;
    void setStartMinimized(bool enabled);

    bool minimizeToTray() const;
    void setMinimizeToTray(bool enabled);

    bool flushCacheAfterApply() const;
    void setFlushCacheAfterApply(bool enabled);

    bool confirmBeforeApply() const;
    void setConfirmBeforeApply(bool enabled);

    QString settingsFilePath() const;
    void resetToDefaults();

signals:
    void themeModeChanged();
    void startWithWindowsChanged();
    void startMinimizedChanged();
    void minimizeToTrayChanged();
    void flushCacheAfterApplyChanged();
    void confirmBeforeApplyChanged();

private:
    void applyStartupEntry();

    QSettings* m_settings = nullptr;
    QString m_themeMode;
    bool m_startWithWindows = false;
    bool m_startMinimized = false;
    bool m_minimizeToTray = true;
    bool m_flushCacheAfterApply = true;
    bool m_confirmBeforeApply = true;
};
