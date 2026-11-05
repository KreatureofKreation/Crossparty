// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "MicrophoneStreams.h"

#include <QTest>

using MuteAction = MicrophoneStreams::MuteAction;

Q_DECLARE_METATYPE(MicrophoneStreams::MuteAction)

// The mute must only ever touch this app's own microphone stream, and must
// hold when the page opens a new one.
class MicrophoneStreamsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void recognisesOnlyOurStreams_data();
    void recognisesOnlyOurStreams();
    void decidesWhatToDo_data();
    void decidesWhatToDo();
};

void MicrophoneStreamsTest::recognisesOnlyOurStreams_data()
{
    QTest::addColumn<QString>("streamAppId");
    QTest::addColumn<QString>("ourAppId");
    QTest::addColumn<bool>("ours");

    const QString ourAppId = QStringLiteral("io.github.KreatureofKreation.Crossparty");

    QTest::newRow("ours") << ourAppId << ourAppId << true;
    QTest::newRow("another flatpak") << QStringLiteral("org.mozilla.firefox") << ourAppId << false;
    QTest::newRow("look-alike") << QStringLiteral("io.github.KreatureofKreation.Crossparty.Spike") << ourAppId << false;
    QTest::newRow("different case") << ourAppId.toLower() << ourAppId << false;
    QTest::newRow("not a flatpak stream") << QString() << ourAppId << false;
    QTest::newRow("we are not in a flatpak") << QString() << QString() << false;
}

void MicrophoneStreamsTest::recognisesOnlyOurStreams()
{
    QFETCH(QString, streamAppId);
    QFETCH(QString, ourAppId);
    QFETCH(bool, ours);

    QCOMPARE(MicrophoneStreams::isOurs(streamAppId, ourAppId), ours);
}

void MicrophoneStreamsTest::decidesWhatToDo_data()
{
    QTest::addColumn<bool>("isNew");
    QTest::addColumn<bool>("streamMuted");
    QTest::addColumn<bool>("userWantsMuted");
    QTest::addColumn<MuteAction>("action");

    QTest::newRow("new stream, user muted") << true << false << true << MuteAction::ApplyChoice;
    QTest::newRow("new stream, user unmuted") << true << false << false << MuteAction::None;
    QTest::newRow("new stream already muted, user unmuted") << true << true << false << MuteAction::ApplyChoice;
    QTest::newRow("known stream matches") << false << true << true << MuteAction::None;
    QTest::newRow("known stream unmuted elsewhere") << false << false << true << MuteAction::AdoptStreamState;
    QTest::newRow("known stream muted elsewhere") << false << true << false << MuteAction::AdoptStreamState;
}

void MicrophoneStreamsTest::decidesWhatToDo()
{
    QFETCH(bool, isNew);
    QFETCH(bool, streamMuted);
    QFETCH(bool, userWantsMuted);
    QFETCH(MuteAction, action);

    QCOMPARE(MicrophoneStreams::decide(isNew, streamMuted, userWantsMuted), action);
}

QTEST_GUILESS_MAIN(MicrophoneStreamsTest)

#include "MicrophoneStreamsTest.moc"
