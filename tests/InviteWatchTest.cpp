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
        QVERIFY(!watch.handleReport(watch.reportPrefix() + QStringLiteral("1")));
    }

    void risingCountIsANewInvite()
    {
        InviteWatch watch;
        watch.handleReport(watch.reportPrefix() + QStringLiteral("0"));
        QVERIFY(watch.handleReport(watch.reportPrefix() + QStringLiteral("1")));
        QVERIFY(watch.handleReport(watch.reportPrefix() + QStringLiteral("2")));
    }

    void sameOrFallingCountIsNot()
    {
        InviteWatch watch;
        watch.handleReport(watch.reportPrefix() + QStringLiteral("2"));
        QVERIFY(!watch.handleReport(watch.reportPrefix() + QStringLiteral("2")));
        QVERIFY(!watch.handleReport(watch.reportPrefix() + QStringLiteral("0")));
    }

    void unknownCountStartsAgain()
    {
        // The page went to its wide layout and back: an invite already
        // waiting must not raise a second toast.
        InviteWatch watch;
        watch.handleReport(watch.reportPrefix() + QStringLiteral("0"));
        watch.handleReport(watch.reportPrefix() + QStringLiteral("none"));
        QVERIFY(!watch.handleReport(watch.reportPrefix() + QStringLiteral("1")));
        QVERIFY(watch.handleReport(watch.reportPrefix() + QStringLiteral("2")));
    }

    void otherMessagesAreIgnored_data()
    {
        // "{report}" stands for this run's real report prefix.
        QTest::addColumn<QString>("message");
        QTest::newRow("page log") << QStringLiteral("Failed to send telemetry.");
        QTest::newRow("not a number") << QStringLiteral("{report}lots");
        QTest::newRow("negative") << QStringLiteral("{report}-1");
        QTest::newRow("too big") << QStringLiteral("{report}100000");
        QTest::newRow("extra text") << QStringLiteral("{report}3 and more");
        QTest::newRow("prefix in the middle") << QStringLiteral("x {report}3");
    }

    void otherMessagesAreIgnored()
    {
        QFETCH(QString, message);
        InviteWatch watch;
        message.replace(QLatin1String("{report}"), watch.reportPrefix());
        watch.handleReport(watch.reportPrefix() + QStringLiteral("0"));
        QVERIFY(!watch.handleReport(message));
        // The known count is untouched, so a real invite still shows.
        QVERIFY(watch.handleReport(watch.reportPrefix() + QStringLiteral("1")));
    }

    void scriptRunsIsolatedOnTheMainPageOnly()
    {
        const InviteWatch watch;
        const QWebEngineScript script = watch.script();
        QCOMPARE(script.worldId(), quint32(QWebEngineScript::ApplicationWorld));
        QVERIFY(!script.runsOnSubFrames());
        QVERIFY(script.sourceCode().contains(QLatin1String("nav-guide")));
        QVERIFY(script.sourceCode().contains(watch.reportPrefix()));
    }

    void reportsWithoutTheTokenAreIgnored()
    {
        // A frame or page script can write console messages too, but cannot
        // know this run's token.
        InviteWatch watch;
        watch.handleReport(watch.reportPrefix() + QStringLiteral("0"));
        QVERIFY(!watch.handleReport(QStringLiteral("crossparty:invites=5")));
        QVERIFY(!watch.handleReport(InviteWatch().reportPrefix() + QStringLiteral("5")));
        // The known count is untouched, so a real invite still shows.
        QVERIFY(watch.handleReport(watch.reportPrefix() + QStringLiteral("1")));
    }

    void eachRunHasItsOwnToken()
    {
        QVERIFY(InviteWatch().reportPrefix() != InviteWatch().reportPrefix());
    }
};

QTEST_GUILESS_MAIN(InviteWatchTest)
#include "InviteWatchTest.moc"
