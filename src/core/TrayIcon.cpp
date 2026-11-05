// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "TrayIcon.h"

#include <QApplication>

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
{
    // A headset from the system icon theme until the app has its own icon.
    m_icon.setIcon(QIcon::fromTheme(QStringLiteral("audio-headset")));
    m_icon.setToolTip(QApplication::applicationDisplayName());

    m_menu.addAction(QIcon::fromTheme(QStringLiteral("window")), tr("Show Crossparty"), this, &TrayIcon::showRequested);
    m_menu.addSeparator();
    m_menu.addAction(QIcon::fromTheme(QStringLiteral("application-exit")), tr("Quit"), qApp, &QCoreApplication::quit);
    m_icon.setContextMenu(&m_menu);

    connect(&m_icon, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);
    if (isAvailable()) {
        m_icon.show();
    }
}

bool TrayIcon::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    // A left click on the icon brings the window back.
    if (reason == QSystemTrayIcon::Trigger) {
        Q_EMIT showRequested();
    }
}
