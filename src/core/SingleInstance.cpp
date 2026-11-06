// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "SingleInstance.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QGuiApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcInstance, "crossparty.instance", QtInfoMsg)

namespace
{
QString objectPath()
{
    // The org.freedesktop.Application convention: the app ID with dots
    // turned into slashes.
    return QLatin1Char('/') + QGuiApplication::desktopFileName().replace(QLatin1Char('.'), QLatin1Char('/'));
}

// On Wayland an app may only bring its window to the front with a token from
// whatever launched it. A later copy passes its own token on, so the running
// copy can use it.
QString activationTokenKey()
{
    return QStringLiteral("activation-token");
}
}

SingleInstance::SingleInstance(QObject *parent)
    : QObject(parent)
{
}

bool SingleInstance::claim()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qCWarning(lcInstance) << "No session bus; cannot check for another running copy";
        return true;
    }

    const QString appId = QGuiApplication::desktopFileName();
    bus.registerObject(objectPath(), this, QDBusConnection::ExportScriptableSlots);
    if (bus.registerService(appId)) {
        return true;
    }
    bus.unregisterObject(objectPath());

    qCInfo(lcInstance) << "Crossparty is already running; showing that window instead";
    QVariantMap platformData;
    const QByteArray token = qgetenv("XDG_ACTIVATION_TOKEN");
    if (!token.isEmpty()) {
        platformData.insert(activationTokenKey(), QString::fromUtf8(token));
    }
    QDBusMessage call = QDBusMessage::createMethodCall(appId, objectPath(), QStringLiteral("org.freedesktop.Application"), QStringLiteral("Activate"));
    call.setArguments({platformData});
    bus.call(call);
    return false;
}

void SingleInstance::Activate(const QVariantMap &platformData)
{
    const QString token = platformData.value(activationTokenKey()).toString();
    if (!token.isEmpty()) {
        // Qt's Wayland support reads this when the window next asks for focus.
        qputenv("XDG_ACTIVATION_TOKEN", token.toUtf8());
    }
    Q_EMIT activationRequested();
}
