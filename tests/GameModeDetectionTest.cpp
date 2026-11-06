// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GameModeDetection.h"

#include <QTest>

class GameModeDetectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void detects_data()
    {
        QTest::addColumn<QStringList>("variables");
        QTest::addColumn<bool>("expected");

        QTest::newRow("plasma desktop") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=KDE")} << false;
        QTest::newRow("nothing set") << QStringList{} << false;
        QTest::newRow("gamescope desktop") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=gamescope")} << true;
        QTest::newRow("gamescope in a list") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=KDE:Gamescope")} << true;
        QTest::newRow("gamescope display only") << QStringList{QStringLiteral("GAMESCOPE_WAYLAND_DISPLAY=gamescope-0")} << true;
        QTest::newRow("empty gamescope display") << QStringList{QStringLiteral("GAMESCOPE_WAYLAND_DISPLAY=")} << false;
        QTest::newRow("name only contains gamescope") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=notgamescope")} << false;
        QTest::newRow("forced on") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=KDE"), QStringLiteral("CROSSPARTY_HANDHELD=1")} << true;
        QTest::newRow("forced off") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=gamescope"), QStringLiteral("CROSSPARTY_HANDHELD=0")} << false;
        QTest::newRow("unknown override ignored") << QStringList{QStringLiteral("XDG_CURRENT_DESKTOP=gamescope"), QStringLiteral("CROSSPARTY_HANDHELD=yes")}
                                                  << true;
    }

    void detects()
    {
        QFETCH(QStringList, variables);
        QFETCH(bool, expected);

        QProcessEnvironment environment;
        for (const QString &variable : std::as_const(variables)) {
            const qsizetype equals = variable.indexOf(QLatin1Char('='));
            environment.insert(variable.left(equals), variable.mid(equals + 1));
        }
        QCOMPARE(GameModeDetection::isGameMode(environment), expected);
    }
};

QTEST_GUILESS_MAIN(GameModeDetectionTest)
#include "GameModeDetectionTest.moc"
