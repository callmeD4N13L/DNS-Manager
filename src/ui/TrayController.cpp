#include "ui/TrayController.hpp"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QQuickWindow>
#include <QSystemTrayIcon>

#include "ui/Backend.hpp"

TrayController::TrayController(Backend* backend, QObject* parent)
    : QObject(parent)
    , m_backend(backend)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    m_tray = new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/app/app-icon-256.png")), this);
    m_tray->setToolTip(QObject::tr("DNS Manager"));

    m_menu = new QMenu;

    QAction* showAction = m_menu->addAction(QObject::tr("Show DNS Manager"));
    connect(showAction, &QAction::triggered, this, &TrayController::showMainWindow);

    m_menu->addSeparator();

    QAction* applyAction = m_menu->addAction(QObject::tr("Apply active profile"));
    connect(applyAction, &QAction::triggered, m_backend, &Backend::applyActive);

    QAction* resetAction = m_menu->addAction(QObject::tr("Reset to DHCP"));
    connect(resetAction, &QAction::triggered, m_backend, &Backend::resetDns);

    QAction* flushAction = m_menu->addAction(QObject::tr("Flush DNS cache"));
    connect(flushAction, &QAction::triggered, m_backend, &Backend::flushDnsCache);

    m_menu->addSeparator();

    QAction* quitAction = m_menu->addAction(QObject::tr("Quit"));
    connect(quitAction, &QAction::triggered, m_backend, &Backend::requestQuit);

    m_tray->setContextMenu(m_menu);
    connect(m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                    showMainWindow();
            });

    m_tray->show();
}

void TrayController::setWindow(QQuickWindow* window)
{
    m_window = window;
}

void TrayController::hideMainWindow()
{
    if (m_window != nullptr)
        m_window->hide();
}

void TrayController::showMainWindow()
{
    if (m_window == nullptr)
        return;
    m_window->show();
    m_window->raise();
    m_window->requestActivate();
}

bool TrayController::isAvailable() const
{
    return m_tray != nullptr;
}
