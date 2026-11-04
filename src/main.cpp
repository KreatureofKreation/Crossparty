// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include <cstdlib>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

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

    return QGuiApplication::exec();
}
