// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QTest>
#include <qtwebenginequickglobal.h>

Q_IMPORT_QML_PLUGIN(CrosspartyPlugin)

// Checks that the app's main interface compiles: no QML syntax errors, and
// every import (Kirigami, QtWebEngine, the app's own module) is found. It
// stops short of creating the window, because that starts Chromium, which
// needs a GPU and a session bus that build sandboxes and CI do not have. The
// running app is checked by hand (docs/plan.md, Manual checklist).
class MainWindowTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void mainInterfaceCompiles();
};

void MainWindowTest::mainInterfaceCompiles()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadFromModule(u"Crossparty", u"Main");

    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
}

// QtWebEngine has to be set up before the application object exists, so this
// test cannot use QTEST_MAIN.
int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();
    QGuiApplication app(argc, argv);
    MainWindowTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "MainWindowTest.moc"
