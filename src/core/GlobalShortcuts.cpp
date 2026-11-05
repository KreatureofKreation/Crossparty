// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GlobalShortcuts.h"

#include "PortalRequest.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcShortcuts, "crossparty.shortcuts", QtInfoMsg)

namespace
{
const char portalService[] = "org.freedesktop.portal.Desktop";
const char portalPath[] = "/org/freedesktop/portal/desktop";
const char shortcutsInterface[] = "org.freedesktop.portal.GlobalShortcuts";

const char sessionToken[] = "crossparty_shortcuts";
const char createSessionToken[] = "crossparty_shortcuts_session";
const char bindToken[] = "crossparty_shortcuts_bind";

// The shortcut IDs are permanent: KDE files the user's chosen keys under
// them.
const char toggleMuteId[] = "toggle-mute";
const char toggleOverlayId[] = "toggle-overlay";

PortalShortcut makeShortcut(const char *shortcutId, const QString &description, const char *preferredKeys)
{
    return {QLatin1String(shortcutId), {{QStringLiteral("description"), description}, {QStringLiteral("preferred_trigger"), QLatin1String(preferredKeys)}}};
}
}

QDBusArgument &operator<<(QDBusArgument &argument, const PortalShortcut &shortcut)
{
    argument.beginStructure();
    argument << shortcut.id << shortcut.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PortalShortcut &shortcut)
{
    argument.beginStructure();
    argument >> shortcut.id >> shortcut.properties;
    argument.endStructure();
    // Qt's D-Bus streaming requires this exact signature.
    return argument; // NOLINT(bugprone-return-const-ref-from-parameter)
}

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<PortalShortcut>();
    qDBusRegisterMetaType<PortalShortcutList>();

    // clang-format off
    QDBusConnection::sessionBus().connect(QLatin1String(portalService), QLatin1String(portalPath), QLatin1String(shortcutsInterface),
                                          QStringLiteral("Activated"), this, SLOT(onActivated(QDBusObjectPath,QString,qulonglong,QVariantMap)));
    // clang-format on

    createSession();
}

PortalShortcutList GlobalShortcuts::requestedShortcuts()
{
    // Key names follow the shortcuts specification: LOGO is the Meta key.
    return {
        makeShortcut(toggleMuteId, tr("Mute or unmute the microphone"), "LOGO+SHIFT+m"),
        makeShortcut(toggleOverlayId, tr("Show or hide the overlay"), "LOGO+SHIFT+x"),
    };
}

void GlobalShortcuts::createSession()
{
    const QVariantMap options{
        {QStringLiteral("handle_token"), QLatin1String(createSessionToken)},
        {QStringLiteral("session_handle_token"), QLatin1String(sessionToken)},
    };
    callPortal(QStringLiteral("CreateSession"), {options}, QLatin1String(createSessionToken), SLOT(onCreateSessionResponse(uint, QVariantMap)));
}

void GlobalShortcuts::onCreateSessionResponse(uint response, const QVariantMap &results)
{
    if (response != 0) {
        qCWarning(lcShortcuts) << "The shortcuts portal refused a session; hotkeys are off";
        return;
    }
    const QVariant handle = results.value(QStringLiteral("session_handle"));
    m_sessionHandle = handle.canConvert<QDBusObjectPath>() ? handle.value<QDBusObjectPath>().path() : handle.toString();
    bindShortcuts();
}

void GlobalShortcuts::bindShortcuts()
{
    // KDE shows its approval dialog only for shortcuts it has not seen
    // before. After that it hands back the keys the user chose.
    const QList<QVariant> arguments{
        QVariant::fromValue(QDBusObjectPath(m_sessionHandle)),
        QVariant::fromValue(requestedShortcuts()),
        QString(),
        QVariantMap{{QStringLiteral("handle_token"), QLatin1String(bindToken)}},
    };
    callPortal(QStringLiteral("BindShortcuts"), arguments, QLatin1String(bindToken), SLOT(onBindShortcutsResponse(uint, QVariantMap)));
}

void GlobalShortcuts::onBindShortcutsResponse(uint response, const QVariantMap &results)
{
    if (response != 0) {
        qCWarning(lcShortcuts) << "Hotkeys were not approved; they can be set later in System Settings";
        return;
    }
    const auto bound = qdbus_cast<PortalShortcutList>(results.value(QStringLiteral("shortcuts")).value<QDBusArgument>());
    for (const PortalShortcut &shortcut : bound) {
        qCInfo(lcShortcuts) << "Hotkey" << shortcut.id << "is" << shortcut.properties.value(QStringLiteral("trigger_description")).toString();
    }
}

void GlobalShortcuts::onActivated(const QDBusObjectPath &sessionHandle, const QString &shortcutId, qulonglong timestamp, const QVariantMap &options)
{
    Q_UNUSED(timestamp)
    Q_UNUSED(options)
    // Every app with a session hears this signal; only ours counts.
    if (sessionHandle.path() != m_sessionHandle) {
        return;
    }
    if (shortcutId == QLatin1String(toggleMuteId)) {
        Q_EMIT muteToggleRequested();
    } else if (shortcutId == QLatin1String(toggleOverlayId)) {
        Q_EMIT overlayToggleRequested();
    }
}

bool GlobalShortcuts::callPortal(const QString &method, const QList<QVariant> &arguments, const QString &token, const char *responseSlot)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    // Listen for the answer before asking, so it cannot be missed.
    bus.connect(QLatin1String(portalService),
                PortalRequest::path(bus.baseService(), token),
                QStringLiteral("org.freedesktop.portal.Request"),
                QStringLiteral("Response"),
                this,
                responseSlot);

    QDBusMessage call = QDBusMessage::createMethodCall(QLatin1String(portalService), QLatin1String(portalPath), QLatin1String(shortcutsInterface), method);
    call.setArguments(arguments);
    const QDBusMessage reply = bus.call(call);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(lcShortcuts) << method << "failed:" << reply.errorMessage();
        return false;
    }
    return true;
}
