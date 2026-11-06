// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "PermissionPolicy.h"

#include <QTest>
#include <QUrl>

using PermissionType = QWebEnginePermission::PermissionType;

// Only Microsoft's own sites get the microphone and notifications, and
// nothing gets anything else.
class PermissionPolicyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void decidesPermissions_data();
    void decidesPermissions();
};

void PermissionPolicyTest::decidesPermissions_data()
{
    QTest::addColumn<PermissionType>("type");
    QTest::addColumn<QString>("origin");
    QTest::addColumn<bool>("allowed");

    const QString xbox = QStringLiteral("https://www.xbox.com/");
    const QString elsewhere = QStringLiteral("https://example.com/");

    QTest::newRow("xbox microphone") << PermissionType::MediaAudioCapture << xbox << true;
    QTest::newRow("xbox notifications") << PermissionType::Notifications << xbox << true;

    QTest::newRow("other site microphone") << PermissionType::MediaAudioCapture << elsewhere << false;
    QTest::newRow("other site notifications") << PermissionType::Notifications << elsewhere << false;
    QTest::newRow("http xbox microphone") << PermissionType::MediaAudioCapture << QStringLiteral("http://www.xbox.com/") << false;

    QTest::newRow("xbox camera") << PermissionType::MediaVideoCapture << xbox << false;
    QTest::newRow("xbox camera and microphone") << PermissionType::MediaAudioVideoCapture << xbox << false;
    QTest::newRow("xbox screen sharing") << PermissionType::DesktopVideoCapture << xbox << false;
    QTest::newRow("xbox screen and audio sharing") << PermissionType::DesktopAudioVideoCapture << xbox << false;
    QTest::newRow("xbox location") << PermissionType::Geolocation << xbox << false;
    QTest::newRow("xbox mouse lock") << PermissionType::MouseLock << xbox << false;
    QTest::newRow("xbox clipboard read") << PermissionType::ClipboardReadWrite << xbox << false;
    QTest::newRow("xbox local fonts") << PermissionType::LocalFontsAccess << xbox << false;
    QTest::newRow("unknown") << PermissionType::Unsupported << xbox << false;
}

void PermissionPolicyTest::decidesPermissions()
{
    QFETCH(PermissionType, type);
    QFETCH(QString, origin);
    QFETCH(bool, allowed);

    QCOMPARE(PermissionPolicy::isAllowed(type, QUrl(origin)), allowed);
}

QTEST_GUILESS_MAIN(PermissionPolicyTest)

#include "PermissionPolicyTest.moc"
