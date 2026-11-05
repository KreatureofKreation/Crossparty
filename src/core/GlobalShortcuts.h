// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QDBusArgument>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>

// One shortcut as the portal expects it on D-Bus: an ID and its properties.
struct PortalShortcut {
    QString id;
    QVariantMap properties;
};
Q_DECLARE_METATYPE(PortalShortcut)
using PortalShortcutList = QList<PortalShortcut>;

QDBusArgument &operator<<(QDBusArgument &argument, const PortalShortcut &shortcut);
const QDBusArgument &operator>>(const QDBusArgument &argument, PortalShortcut &shortcut);

// System-wide hotkeys through KDE's shortcuts portal, so they work inside
// games. KDE asks the user to approve them the first time, and remembers the
// keys after that. No Flatpak permission is needed.
//
// Lessons from the spike (docs/spike.md): key names use LOGO, not SUPER; and
// every run has to open its own portal session before any key reaches the
// app.
class GlobalShortcuts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit GlobalShortcuts(QObject *parent = nullptr);

    // The shortcuts the app asks for, with KDE's preferred keys.
    static PortalShortcutList requestedShortcuts();

private:
    void createSession();
    void bindShortcuts();
    bool callPortal(const QString &method, const QList<QVariant> &arguments, const QString &token, const char *responseSlot);

    QString m_sessionHandle;

Q_SIGNALS:
    void muteToggleRequested();
    void overlayToggleRequested();

private Q_SLOTS:
    void onCreateSessionResponse(uint response, const QVariantMap &results);
    void onBindShortcutsResponse(uint response, const QVariantMap &results);
    void onActivated(const QDBusObjectPath &sessionHandle, const QString &shortcutId, qulonglong timestamp, const QVariantMap &options);
};
