// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QList>
#include <Qt>

#include <optional>

// Turns controller input into the keys Crossparty's own controls and
// Microsoft's page already understand: the D-pad and left stick become arrow
// keys, A becomes Enter and B becomes Escape. A held direction repeats, like
// a held key. It only decides which keys; GamepadInput reads the controller
// and sends them, and only while Crossparty has focus. See
// docs/security.md#sec-024.
//
// Plain logic with a clock passed in, so the tests can check every decision.
class GamepadNavigator
{
public:
    enum class Button : quint8 {
        Up,
        Down,
        Left,
        Right,
        Accept,
        Back,
    };

    // A held direction first repeats after this long, then this often.
    static constexpr qint64 repeatDelayMs = 400;
    static constexpr qint64 repeatIntervalMs = 120;
    // The left stick counts as pushed past this far, and as let go below
    // this, so it never flickers around a single threshold.
    static constexpr double stickPushed = 0.6;
    static constexpr double stickLetGo = 0.3;

    // Each of these returns the keys to send now, in order.
    QList<Qt::Key> press(Button button, qint64 nowMs);
    void release(Button button);
    // Left stick position, each axis from -1 to 1: across is positive to the
    // right, down is positive downwards.
    QList<Qt::Key> moveStick(double across, double down, qint64 nowMs);
    // Called regularly: repeats a held direction when it is due.
    QList<Qt::Key> tick(qint64 nowMs);
    // Forgets everything held, for example when Crossparty loses focus.
    void reset();

private:
    QList<Qt::Key> hold(Qt::Key key, std::optional<Button> button, qint64 nowMs);

    // The direction being held, if any, and what holds it: a D-pad button, or
    // the stick when there is no button.
    std::optional<Qt::Key> m_heldKey;
    std::optional<Button> m_heldButton;
    qint64 m_nextRepeatMs = 0;
};
