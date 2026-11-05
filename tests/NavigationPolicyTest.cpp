// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "NavigationPolicy.h"

#include <QTest>
#include <QUrl>

using Decision = NavigationPolicy::Decision;

Q_DECLARE_METATYPE(NavigationPolicy::Decision)

// Where each kind of link ends up: in the app, in the user's browser, or
// nowhere.
class NavigationPolicyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void routesPageNavigation_data();
    void routesPageNavigation();
    void letsEmbeddedFramesLoad();
};

void NavigationPolicyTest::routesPageNavigation_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<Decision>("decision");

    QTest::newRow("xbox") << QStringLiteral("https://www.xbox.com/play") << Decision::LoadInApp;
    QTest::newRow("sign-in") << QStringLiteral("https://login.live.com/") << Decision::LoadInApp;

    // Links shared in party chat go to the user's own browser.
    QTest::newRow("other https site") << QStringLiteral("https://example.com/video") << Decision::OpenInBrowser;
    QTest::newRow("other microsoft site") << QStringLiteral("https://support.xbox.com/") << Decision::OpenInBrowser;
    QTest::newRow("look-alike host") << QStringLiteral("https://www.xbox.com.attacker.example/") << Decision::OpenInBrowser;

    // Never handed to the desktop: these could open local files or start
    // other programs.
    QTest::newRow("plain http") << QStringLiteral("http://example.com/") << Decision::Block;
    QTest::newRow("local file") << QStringLiteral("file:///home/") << Decision::Block;
    QTest::newRow("javascript") << QStringLiteral("javascript:alert(1)") << Decision::Block;
    QTest::newRow("data") << QStringLiteral("data:text/html,hello") << Decision::Block;
    QTest::newRow("app link scheme") << QStringLiteral("steam://run/1") << Decision::Block;
    QTest::newRow("mail link") << QStringLiteral("mailto:someone@example.com") << Decision::Block;
    QTest::newRow("empty") << QString() << Decision::Block;
}

void NavigationPolicyTest::routesPageNavigation()
{
    QFETCH(QString, address);
    QFETCH(Decision, decision);

    QCOMPARE(NavigationPolicy::decideForPage(QUrl(address)), decision);
}

void NavigationPolicyTest::letsEmbeddedFramesLoad()
{
    QCOMPARE(NavigationPolicy::decideForEmbeddedFrame(QUrl(QStringLiteral("https://example.com/frame"))), Decision::LoadInApp);
}

QTEST_GUILESS_MAIN(NavigationPolicyTest)

#include "NavigationPolicyTest.moc"
