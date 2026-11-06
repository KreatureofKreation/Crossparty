// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "InviteWatch.h"

#include <QTest>

class InviteWatchTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void firstCountIsOnlyAStartingPoint()
    {
        InviteWatch watch;
        QVERIFY(!watch.handleReport(QStringLiteral("crossparty:invites=1")));
    }

    void risingCountIsANewInvite()
    {
        InviteWatch watch;
        watch.handleReport(QStringLiteral("crossparty:invites=0"));
        QVERIFY(watch.handleReport(QStringLiteral("crossparty:invites=1")));
        QVERIFY(watch.handleReport(QStringLiteral("crossparty:invites=2")));
    }

    void sameOrFallingCountIsNot()
    {
        InviteWatch watch;
        watch.handleReport(QStringLiteral("crossparty:invites=2"));
        QVERIFY(!watch.handleReport(QStringLiteral("crossparty:invites=2")));
        QVERIFY(!watch.handleReport(QStringLiteral("crossparty:invites=0")));
    }

    void unknownCountStartsAgain()
    {
        // The page went to its wide layout and back: an invite already
        // waiting must not raise a second toast.
        InviteWatch watch;
        watch.handleReport(QStringLiteral("crossparty:invites=0"));
        watch.handleReport(QStringLiteral("crossparty:invites=none"));
        QVERIFY(!watch.handleReport(QStringLiteral("crossparty:invites=1")));
        QVERIFY(watch.handleReport(QStringLiteral("crossparty:invites=2")));
    }

    void otherMessagesAreIgnored_data()
    {
        QTest::addColumn<QString>("message");
        QTest::newRow("page log") << QStringLiteral("Failed to send telemetry.");
        QTest::newRow("not a number") << QStringLiteral("crossparty:invites=lots");
        QTest::newRow("negative") << QStringLiteral("crossparty:invites=-1");
        QTest::newRow("too big") << QStringLiteral("crossparty:invites=100000");
        QTest::newRow("extra text") << QStringLiteral("crossparty:invites=3 and more");
        QTest::newRow("prefix in the middle") << QStringLiteral("x crossparty:invites=3");
    }

    void otherMessagesAreIgnored()
    {
        QFETCH(QString, message);
        InviteWatch watch;
        watch.handleReport(QStringLiteral("crossparty:invites=0"));
        QVERIFY(!watch.handleReport(message));
        // The known count is untouched, so a real invite still shows.
        QVERIFY(watch.handleReport(QStringLiteral("crossparty:invites=1")));
    }

    void scriptRunsIsolatedOnTheMainPageOnly()
    {
        const QWebEngineScript script = InviteWatch::script();
        QCOMPARE(script.worldId(), quint32(QWebEngineScript::ApplicationWorld));
        QVERIFY(!script.runsOnSubFrames());
        QVERIFY(script.sourceCode().contains(QLatin1String("nav-guide")));
    }
};

QTEST_GUILESS_MAIN(InviteWatchTest)
#include "InviteWatchTest.moc"
