// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "PortalRequest.h"

#include <QTest>

// If the path is wrong, the app never hears the portal's answer: no saved
// sign-in and no hotkeys, with no error to show why.
class PortalRequestTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void buildsTheResponsePath();
};

void PortalRequestTest::buildsTheResponsePath()
{
    QCOMPARE(PortalRequest::path(QStringLiteral(":1.42"), QStringLiteral("crossparty_secret")),
             QStringLiteral("/org/freedesktop/portal/desktop/request/1_42/crossparty_secret"));
    QCOMPARE(PortalRequest::path(QStringLiteral(":1.2172"), QStringLiteral("token")), QStringLiteral("/org/freedesktop/portal/desktop/request/1_2172/token"));
}

QTEST_GUILESS_MAIN(PortalRequestTest)

#include "PortalRequestTest.moc"
