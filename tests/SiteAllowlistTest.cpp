// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "SiteAllowlist.h"

#include <QTest>
#include <QUrl>

// The allowlist decides which sites may load inside the app, so it is tested
// against the tricks a malicious link in party chat might use.
class SiteAllowlistTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void decidesWhatLoadsInTheApp_data();
    void decidesWhatLoadsInTheApp();
    void listsOnlyExactHosts();
    void startPageIsAllowed();
};

void SiteAllowlistTest::decidesWhatLoadsInTheApp_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("allowed");

    // The sites the app needs.
    QTest::newRow("xbox play") << QStringLiteral("https://www.xbox.com/play") << true;
    QTest::newRow("xbox localised") << QStringLiteral("https://www.xbox.com/en-AU/play") << true;
    QTest::newRow("xbox play host") << QStringLiteral("https://play.xbox.com/") << true;
    QTest::newRow("microsoft sign-in") << QStringLiteral("https://login.live.com/oauth20_authorize.srf?x=1") << true;
    QTest::newRow("work sign-in redirect") << QStringLiteral("https://login.microsoftonline.com/") << true;
    QTest::newRow("account checks") << QStringLiteral("https://account.live.com/proofs") << true;
    QTest::newRow("upper case host") << QStringLiteral("https://WWW.XBOX.COM/play") << true;
    QTest::newRow("explicit https port") << QStringLiteral("https://www.xbox.com:443/play") << true;

    // Right host, unsafe in some other way.
    QTest::newRow("plain http") << QStringLiteral("http://www.xbox.com/play") << false;
    QTest::newRow("other port") << QStringLiteral("https://www.xbox.com:8443/play") << false;
    QTest::newRow("user name hides real host") << QStringLiteral("https://www.xbox.com@attacker.example/") << false;
    QTest::newRow("user name on real host") << QStringLiteral("https://someone@www.xbox.com/") << false;
    QTest::newRow("websocket scheme") << QStringLiteral("wss://www.xbox.com/") << false;

    // Look-alike and neighbouring hosts.
    QTest::newRow("allowed name as prefix") << QStringLiteral("https://www.xbox.com.attacker.example/") << false;
    QTest::newRow("allowed name as suffix") << QStringLiteral("https://attackerwww.xbox.com/") << false;
    QTest::newRow("bare domain") << QStringLiteral("https://xbox.com/play") << false;
    QTest::newRow("other subdomain") << QStringLiteral("https://support.xbox.com/") << false;
    QTest::newRow("trailing dot") << QStringLiteral("https://www.xbox.com./play") << false;
    QTest::newRow("look-alike letters") << QStringLiteral("https://www.xbоx.com/") << false;
    QTest::newRow("unrelated site") << QStringLiteral("https://example.com/") << false;

    // Not web addresses at all.
    QTest::newRow("local file") << QStringLiteral("file:///etc/passwd") << false;
    QTest::newRow("javascript") << QStringLiteral("javascript:alert(1)") << false;
    QTest::newRow("data") << QStringLiteral("data:text/html,hello") << false;
    QTest::newRow("blank page") << QStringLiteral("about:blank") << false;
    QTest::newRow("browser internals") << QStringLiteral("chrome://settings") << false;
    QTest::newRow("empty") << QString() << false;
}

void SiteAllowlistTest::decidesWhatLoadsInTheApp()
{
    QFETCH(QString, address);
    QFETCH(bool, allowed);

    QCOMPARE(SiteAllowlist::isAllowedInApp(QUrl(address)), allowed);
}

void SiteAllowlistTest::listsOnlyExactHosts()
{
    const QStringList hosts = SiteAllowlist::allowedHosts();
    for (const QString &host : hosts) {
        QVERIFY2(!host.contains(QLatin1Char('*')), qPrintable(host));
        QCOMPARE(host, host.toLower());
    }
}

void SiteAllowlistTest::startPageIsAllowed()
{
    // The app's first page must load inside the app, never in the browser.
    QVERIFY(SiteAllowlist::isAllowedInApp(SiteAllowlist::startUrl()));
}

QTEST_GUILESS_MAIN(SiteAllowlistTest)

#include "SiteAllowlistTest.moc"
