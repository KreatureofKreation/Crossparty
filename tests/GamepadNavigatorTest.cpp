// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GamepadNavigator.h"

#include <QTest>

using Button = GamepadNavigator::Button;
using Keys = QList<Qt::Key>;

// The controller only ever becomes a few ordinary keys, and a held direction
// repeats the way a held key does.
class GamepadNavigatorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void buttonsBecomeKeys_data()
    {
        QTest::addColumn<Button>("button");
        QTest::addColumn<Qt::Key>("key");
        QTest::newRow("up") << Button::Up << Qt::Key_Up;
        QTest::newRow("down") << Button::Down << Qt::Key_Down;
        QTest::newRow("left") << Button::Left << Qt::Key_Left;
        QTest::newRow("right") << Button::Right << Qt::Key_Right;
        QTest::newRow("A is Enter") << Button::Accept << Qt::Key_Return;
        QTest::newRow("B is Escape") << Button::Back << Qt::Key_Escape;
    }

    void buttonsBecomeKeys()
    {
        QFETCH(Button, button);
        QFETCH(Qt::Key, key);
        GamepadNavigator navigator;
        QCOMPARE(navigator.press(button, 0), Keys{key});
    }

    void heldDirectionRepeats()
    {
        GamepadNavigator navigator;
        navigator.press(Button::Down, 0);
        QVERIFY(navigator.tick(GamepadNavigator::repeatDelayMs - 1).isEmpty());
        QCOMPARE(navigator.tick(GamepadNavigator::repeatDelayMs), Keys{Qt::Key_Down});
        const qint64 next = GamepadNavigator::repeatDelayMs + GamepadNavigator::repeatIntervalMs;
        QVERIFY(navigator.tick(next - 1).isEmpty());
        QCOMPARE(navigator.tick(next), Keys{Qt::Key_Down});
    }

    void letGoStopsRepeating()
    {
        GamepadNavigator navigator;
        navigator.press(Button::Down, 0);
        navigator.release(Button::Down);
        QVERIFY(navigator.tick(10000).isEmpty());
    }

    void enterAndEscapeNeverRepeat()
    {
        GamepadNavigator navigator;
        navigator.press(Button::Accept, 0);
        navigator.press(Button::Back, 0);
        QVERIFY(navigator.tick(10000).isEmpty());
    }

    void stickNeedsAFirmPush()
    {
        GamepadNavigator navigator;
        QVERIFY(navigator.moveStick(0.5, 0, 0).isEmpty());
        QCOMPARE(navigator.moveStick(0.7, 0, 0), Keys{Qt::Key_Right});
        // Held, not pressed again, while it stays out.
        QVERIFY(navigator.moveStick(0.9, 0, 10).isEmpty());
    }

    void stickDoesNotFlickerNearTheEdge()
    {
        GamepadNavigator navigator;
        navigator.moveStick(0.7, 0, 0);
        // Between "let go" and "pushed": still held, nothing new.
        QVERIFY(navigator.moveStick(0.4, 0, 10).isEmpty());
        QCOMPARE(navigator.tick(GamepadNavigator::repeatDelayMs), Keys{Qt::Key_Right});
        navigator.moveStick(0.1, 0, 500);
        QVERIFY(navigator.tick(10000).isEmpty());
    }

    void stickFollowsItsMainDirection()
    {
        GamepadNavigator navigator;
        QCOMPARE(navigator.moveStick(0.3, -0.8, 0), Keys{Qt::Key_Up});
        QCOMPARE(navigator.moveStick(-0.9, -0.2, 10), Keys{Qt::Key_Left});
    }

    void dpadOverrulesTheStick()
    {
        GamepadNavigator navigator;
        navigator.press(Button::Up, 0);
        QVERIFY(navigator.moveStick(0.9, 0, 10).isEmpty());
        QCOMPARE(navigator.tick(GamepadNavigator::repeatDelayMs), Keys{Qt::Key_Up});
    }

    void resetForgetsWhatWasHeld()
    {
        // Losing focus must not leave a direction repeating when focus returns.
        GamepadNavigator navigator;
        navigator.press(Button::Left, 0);
        navigator.reset();
        QVERIFY(navigator.tick(10000).isEmpty());
    }
};

QTEST_GUILESS_MAIN(GamepadNavigatorTest)

#include "GamepadNavigatorTest.moc"
