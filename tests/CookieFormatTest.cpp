// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "CookieFormat.h"

#include <QTest>

// The saved cookies have to come back exactly as Chromium expects them, or
// the user is signed out after every restart.
class CookieFormatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void roundTrips();
    void dropsExpiredCookies();
    void keepsSessionCookies();
    void ignoresOtherFormats();
    void skipsUnreadableLines();
    void restoresHostOnlyCookiesWithoutADomain();
    void keepsTheDomainOfDomainCookies();
    void findsTheOrigin();
};

namespace
{
QDateTime now()
{
    return QDateTime(QDate(2026, 11, 5), QTime(12, 0), QTimeZone::UTC);
}

QNetworkCookie makeCookie(const QByteArray &name, const QString &domain, const QDateTime &expires = {})
{
    // Shaped like a real token: base64 with padding. Cookie values cannot
    // contain ";" at all, so that is not tested.
    QNetworkCookie cookie(name, QByteArrayLiteral("dGVzdA==/+.token-value"));
    cookie.setDomain(domain);
    cookie.setPath(QStringLiteral("/"));
    cookie.setSecure(true);
    cookie.setHttpOnly(true);
    cookie.setExpirationDate(expires);
    return cookie;
}
}

void CookieFormatTest::roundTrips()
{
    const QNetworkCookie cookie = makeCookie("MSPAuth", QStringLiteral(".live.com"), now().addDays(30));
    const QList<QNetworkCookie> restored = CookieFormat::fromBytes(CookieFormat::toBytes({cookie}, now()));

    QCOMPARE(restored.size(), 1);
    QCOMPARE(restored.constFirst().name(), cookie.name());
    QCOMPARE(restored.constFirst().value(), cookie.value());
    QCOMPARE(restored.constFirst().domain(), cookie.domain());
    QCOMPARE(restored.constFirst().path(), cookie.path());
    QVERIFY(restored.constFirst().isSecure());
    QVERIFY(restored.constFirst().isHttpOnly());
    QCOMPARE(restored.constFirst().expirationDate(), cookie.expirationDate());
}

void CookieFormatTest::dropsExpiredCookies()
{
    const QList<QNetworkCookie> cookies{
        makeCookie("old", QStringLiteral(".xbox.com"), now().addSecs(-1)),
        makeCookie("current", QStringLiteral(".xbox.com"), now().addDays(1)),
    };
    const QList<QNetworkCookie> restored = CookieFormat::fromBytes(CookieFormat::toBytes(cookies, now()));

    QCOMPARE(restored.size(), 1);
    QCOMPARE(restored.constFirst().name(), QByteArrayLiteral("current"));
}

void CookieFormatTest::keepsSessionCookies()
{
    // The spike showed sign-in needs them (docs/spike.md).
    const QNetworkCookie session = makeCookie("uaid", QStringLiteral("login.live.com"));
    QVERIFY(session.isSessionCookie());
    QCOMPARE(CookieFormat::fromBytes(CookieFormat::toBytes({session}, now())).size(), 1);
}

void CookieFormatTest::ignoresOtherFormats()
{
    QVERIFY(CookieFormat::fromBytes(QByteArray()).isEmpty());
    QVERIFY(CookieFormat::fromBytes(QByteArrayLiteral("crossparty-cookies 2\nAAAA")).isEmpty());
    QVERIFY(CookieFormat::fromBytes(QByteArrayLiteral("name=value")).isEmpty());
}

void CookieFormatTest::skipsUnreadableLines()
{
    const QByteArray good = makeCookie("good", QStringLiteral(".xbox.com")).toRawForm().toBase64();
    const QByteArray bytes = QByteArrayLiteral("crossparty-cookies 1\n***not base64***\n") + good;

    const QList<QNetworkCookie> restored = CookieFormat::fromBytes(bytes);
    QCOMPARE(restored.size(), 1);
    QCOMPARE(restored.constFirst().name(), QByteArrayLiteral("good"));
}

void CookieFormatTest::restoresHostOnlyCookiesWithoutADomain()
{
    // Regression test for the spike bug: with a domain set, Chromium refused
    // Microsoft's "__Host-" sign-in cookie and the user was signed out.
    const QNetworkCookie hostOnly = makeCookie("__Host-MSAAUTHP", QStringLiteral("login.live.com"), now().addDays(30));
    QVERIFY(CookieFormat::prepareForRestore(hostOnly).domain().isEmpty());
}

void CookieFormatTest::keepsTheDomainOfDomainCookies()
{
    const QNetworkCookie domainCookie = makeCookie("MSPAuth", QStringLiteral(".live.com"), now().addDays(30));
    QCOMPARE(CookieFormat::prepareForRestore(domainCookie).domain(), QStringLiteral(".live.com"));
}

void CookieFormatTest::findsTheOrigin()
{
    QCOMPARE(CookieFormat::originFor(makeCookie("a", QStringLiteral(".live.com"))), QUrl(QStringLiteral("https://live.com")));
    QCOMPARE(CookieFormat::originFor(makeCookie("b", QStringLiteral("login.live.com"))), QUrl(QStringLiteral("https://login.live.com")));
}

QTEST_GUILESS_MAIN(CookieFormatTest)

#include "CookieFormatTest.moc"
