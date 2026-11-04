// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTest>

// Checks that the main interface file loads and creates a window. This catches
// QML syntax errors and missing imports (such as Kirigami not being installed)
// before they reach a release.
class MainWindowTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadsAndCreatesAWindow();
};

void MainWindowTest::loadsAndCreatesAWindow()
{
    QQmlApplicationEngine engine;
    engine.load(QUrl::fromLocalFile(QStringLiteral(CROSSTALK_MAIN_QML)));

    QCOMPARE(engine.rootObjects().size(), 1);
    QVERIFY(qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst()) != nullptr);
}

QTEST_MAIN(MainWindowTest)

#include "MainWindowTest.moc"
