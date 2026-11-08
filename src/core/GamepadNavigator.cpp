// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GamepadNavigator.h"

#include <cmath>

namespace
{
std::optional<Qt::Key> directionKey(GamepadNavigator::Button button)
{
    switch (button) {
    case GamepadNavigator::Button::Up:
        return Qt::Key_Up;
    case GamepadNavigator::Button::Down:
        return Qt::Key_Down;
    case GamepadNavigator::Button::Left:
        return Qt::Key_Left;
    case GamepadNavigator::Button::Right:
        return Qt::Key_Right;
    case GamepadNavigator::Button::Accept:
    case GamepadNavigator::Button::Back:
        return std::nullopt;
    }
    return std::nullopt;
}

// The stick points mostly one way; that way wins.
Qt::Key stickKey(double across, double down)
{
    if (std::abs(across) >= std::abs(down)) {
        return across > 0 ? Qt::Key_Right : Qt::Key_Left;
    }
    return down > 0 ? Qt::Key_Down : Qt::Key_Up;
}
}

QList<Qt::Key> GamepadNavigator::press(Button button, qint64 nowMs)
{
    if (button == Button::Accept) {
        return {Qt::Key_Return};
    }
    if (button == Button::Back) {
        return {Qt::Key_Escape};
    }
    const std::optional<Qt::Key> key = directionKey(button);
    if (!key) {
        return {};
    }
    return hold(*key, button, nowMs);
}

void GamepadNavigator::release(Button button)
{
    if (m_heldButton == button) {
        reset();
    }
}

QList<Qt::Key> GamepadNavigator::moveStick(double across, double down, qint64 nowMs)
{
    if (m_heldKey && m_heldButton) {
        // The D-pad is in charge until it is let go.
        return {};
    }
    const double distance = std::max(std::abs(across), std::abs(down));
    if (m_heldKey) {
        if (distance < stickLetGo) {
            reset();
            return {};
        }
        const Qt::Key key = stickKey(across, down);
        if (key != *m_heldKey && distance >= stickPushed) {
            return hold(key, std::nullopt, nowMs);
        }
        return {};
    }
    if (distance >= stickPushed) {
        return hold(stickKey(across, down), std::nullopt, nowMs);
    }
    return {};
}

QList<Qt::Key> GamepadNavigator::tick(qint64 nowMs)
{
    if (!m_heldKey || nowMs < m_nextRepeatMs) {
        return {};
    }
    m_nextRepeatMs = nowMs + repeatIntervalMs;
    return {*m_heldKey};
}

void GamepadNavigator::reset()
{
    m_heldKey.reset();
    m_heldButton.reset();
    m_nextRepeatMs = 0;
}

QList<Qt::Key> GamepadNavigator::hold(Qt::Key key, std::optional<Button> button, qint64 nowMs)
{
    m_heldKey = key;
    m_heldButton = button;
    m_nextRepeatMs = nowMs + repeatDelayMs;
    return {key};
}
