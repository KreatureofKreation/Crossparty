// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GamepadInput.h"
#include "SingleInstance.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlExtensionPlugin>
#include <QQuickStyle>
#include <qtwebenginequickglobal.h>

#include <cstdlib>

// The interface is a static QML module linked into the app.
Q_IMPORT_QML_PLUGIN(CrosspartyPlugin)

int main(int argc, char *argv[])
{
    // Thin scroll bars that sit over the page and fade out when not in use,
    // like Plasma's own. This is a browser setting, not a change to
    // Microsoft's page. Any flags the user set themselves are kept.
    QByteArray chromiumFlags = qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");
    chromiumFlags.append(" --enable-features=FluentOverlayScrollbar");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", chromiumFlags);

    // QtWebEngine must be set up before the application object exists.
    QtWebEngineQuick::initialize();

    // QApplication rather than QGuiApplication: the tray menu is a classic
    // Qt menu.
    QApplication app(argc, argv);

    QGuiApplication::setApplicationName(QStringLiteral("crossparty"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Crossparty"));
    QGuiApplication::setApplicationVersion(QStringLiteral(CROSSPARTY_VERSION));

    // The desktop file name becomes our Wayland app ID. KDE's portals use it to
    // tie each permission you grant (shortcuts, notifications) to this app
    // only. See docs/security.md#sec-006.
    QGuiApplication::setDesktopFileName(QStringLiteral(CROSSPARTY_APP_ID));

    // Use Plasma's own control style so the app looks like it shipped with
    // KDE, unless the user has chosen a style themselves.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    // Never a second party connection: if Crossparty is already running,
    // bring its window to the front and stop here.
    SingleInstance singleInstance;
    if (!singleInstance.claim()) {
        return EXIT_SUCCESS;
    }

    QQmlApplicationEngine engine;

    // Fail safe: if the interface cannot load, quit with an error rather than
    // keep running as an invisible process. See docs/security.md#sec-007.
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] {
            QCoreApplication::exit(EXIT_FAILURE);
        },
        Qt::QueuedConnection);

    engine.loadFromModule(u"Crossparty", u"Main");

    // A controller drives Crossparty while it is focused, with no Steam
    // Input setup. See docs/security.md#sec-024.
    const GamepadInput gamepadInput;

    // Opening Crossparty again, or activating it from the desktop, shows the
    // window, even when it is hidden in the tray.
    QObject::connect(&singleInstance, &SingleInstance::activationRequested, &engine, [&engine] {
        if (!engine.rootObjects().isEmpty()) {
            QMetaObject::invokeMethod(engine.rootObjects().constFirst(), "bringToFront");
        }
    });

    return QApplication::exec();
}
