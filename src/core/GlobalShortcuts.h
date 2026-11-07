// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QDBusArgument>
#include <QDBusMessage>
#include <QHash>
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

    // The keys the user actually has for each hotkey, as KDE describes them
    // (for example "Meta+Shift+M"). Empty when the hotkey is not set, for
    // example when the user declined it, so hints never show keys that do
    // nothing.
    Q_PROPERTY(QString muteKeys READ muteKeys NOTIFY keysChanged)
    Q_PROPERTY(QString overlayKeys READ overlayKeys NOTIFY keysChanged)

public:
    explicit GlobalShortcuts(QObject *parent = nullptr);

    // The shortcuts the app asks for, with KDE's preferred keys.
    static PortalShortcutList requestedShortcuts();

    QString muteKeys() const;
    QString overlayKeys() const;

private:
    void createSession();
    void bindShortcuts();
    bool callPortal(const QString &method, const QList<QVariant> &arguments, const QString &token, const char *responseSlot);

    void updateKeys(const PortalShortcutList &shortcuts);

    QString m_sessionHandle;
    // Shortcut ID to the keys KDE reports for it.
    QHash<QString, QString> m_keys;

Q_SIGNALS:
    void muteToggleRequested();
    void overlayToggleRequested();
    void keysChanged();

private Q_SLOTS:
    void onCreateSessionResponse(uint response, const QVariantMap &results);
    void onBindShortcutsResponse(uint response, const QVariantMap &results);
    void onActivated(const QDBusObjectPath &sessionHandle, const QString &shortcutId, qulonglong timestamp, const QVariantMap &options);
    // The user changed the keys in System Settings.
    void onShortcutsChanged(const QDBusMessage &message);
};
