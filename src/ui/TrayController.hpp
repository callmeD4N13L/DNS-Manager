#pragma once

#include <QObject>

class Backend;
class QMenu;
class QQuickWindow;
class QSystemTrayIcon;

// System tray icon + context menu. Owned by Backend; the main window pointer
// is attached later via setWindow() once QML has created it. Requires a
// QApplication (QMenu is a widgets class).
class TrayController : public QObject
{
    Q_OBJECT

public:
    explicit TrayController(Backend* backend, QObject* parent = nullptr);

    void setWindow(QQuickWindow* window);
    void hideMainWindow();
    void showMainWindow();

    bool isAvailable() const;

private:
    Backend* m_backend = nullptr;
    QSystemTrayIcon* m_tray = nullptr;
    QMenu* m_menu = nullptr;
    QQuickWindow* m_window = nullptr;
};
