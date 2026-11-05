// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QMenu>
#include <QObject>
#include <QQmlEngine>
#include <QSystemTrayIcon>

// The icon in KDE's system tray. Closing the window sends the app here, and
// Quit lives in its menu.
class TrayIcon : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // False when the desktop has no tray. Closing the window then quits the
    // app, so it can never be left running with no way back to it.
    Q_PROPERTY(bool available READ isAvailable CONSTANT)

public:
    explicit TrayIcon(QObject *parent = nullptr);

    bool isAvailable() const;

Q_SIGNALS:
    void showRequested();

private:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

    QMenu m_menu;
    QSystemTrayIcon m_icon;
};
