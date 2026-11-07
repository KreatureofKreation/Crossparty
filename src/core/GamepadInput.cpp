// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GamepadInput.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QQuickItem>
#include <QQuickWindow>
#include <QWindow>

#include <SDL3/SDL.h>

#include <chrono>
#include <optional>

Q_LOGGING_CATEGORY(lcGamepad, "crossparty.gamepad", QtInfoMsg)

namespace
{
// About once a frame: quick enough to feel instant, and only while active.
constexpr std::chrono::milliseconds pollInterval{16};
constexpr double axisRange = 32767.0;

std::optional<GamepadNavigator::Button> buttonFor(Uint8 button)
{
    // By position, not label: the bottom face button accepts and the right
    // one goes back, as on Xbox and Steam Deck.
    switch (button) {
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
        return GamepadNavigator::Button::Up;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
        return GamepadNavigator::Button::Down;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
        return GamepadNavigator::Button::Left;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
        return GamepadNavigator::Button::Right;
    case SDL_GAMEPAD_BUTTON_SOUTH:
        return GamepadNavigator::Button::Accept;
    case SDL_GAMEPAD_BUTTON_EAST:
        return GamepadNavigator::Button::Back;
    default:
        // Everything else, including the Guide button, which stays Steam's.
        return std::nullopt;
    }
}

// The party page, if it is what has keyboard focus (PartyPage.qml).
QObject *focusedPartyPage(QWindow *window)
{
    const auto *quickWindow = qobject_cast<QQuickWindow *>(window);
    for (QQuickItem *item = quickWindow ? quickWindow->activeFocusItem() : nullptr; item; item = item->parentItem()) {
        if (item->objectName() == QLatin1String("partyPage")) {
            return item;
        }
    }
    return nullptr;
}

void openGamepad(SDL_JoystickID which)
{
    if (SDL_Gamepad *gamepad = SDL_OpenGamepad(which)) {
        qCInfo(lcGamepad) << "Controller connected:" << SDL_GetGamepadName(gamepad);
    }
}

bool isActive()
{
    return QGuiApplication::applicationState() == Qt::ApplicationActive && QGuiApplication::focusWindow();
}
}

GamepadInput::GamepadInput(QObject *parent)
    : QObject(parent)
{
    // Controllers are read through /dev/input only. The app has no access to
    // raw USB devices (hidraw), so SDL is told not to look for them.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "0");
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        qCWarning(lcGamepad) << "Controllers are off:" << SDL_GetError();
        return;
    }
    m_started = true;
    m_clock.start();

    m_pollTimer.setInterval(pollInterval);
    connect(&m_pollTimer, &QTimer::timeout, this, &GamepadInput::poll);
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &GamepadInput::onApplicationStateChanged);
    onApplicationStateChanged(QGuiApplication::applicationState());
}

GamepadInput::~GamepadInput()
{
    if (m_started) {
        SDL_Quit();
    }
}

void GamepadInput::onApplicationStateChanged(Qt::ApplicationState state)
{
    m_navigator.reset();
    if (state != Qt::ApplicationActive) {
        // Not reading controllers at all while another app is in front.
        m_pollTimer.stop();
        return;
    }
    // Everything pressed while Crossparty was not active is read now and
    // thrown away; only controllers coming and going are kept track of.
    SDL_PumpEvents();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
            openGamepad(event.gdevice.which);
        } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
            SDL_CloseGamepad(SDL_GetGamepadFromID(event.gdevice.which));
        }
    }
    m_pollTimer.start();
}

void GamepadInput::poll()
{
    // Belt and braces: never send anything without a focused window.
    const bool active = isActive();
    const qint64 now = m_clock.elapsed();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            openGamepad(event.gdevice.which);
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            SDL_CloseGamepad(SDL_GetGamepadFromID(event.gdevice.which));
            m_navigator.reset();
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if (const auto button = buttonFor(event.gbutton.button); button && active) {
                sendKeys(m_navigator.press(*button, now));
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (const auto button = buttonFor(event.gbutton.button)) {
                m_navigator.release(*button);
            }
            break;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if (active && (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX || event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY)) {
                SDL_Gamepad *gamepad = SDL_GetGamepadFromID(event.gaxis.which);
                const double across = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / axisRange;
                const double down = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / axisRange;
                sendKeys(m_navigator.moveStick(across, down, now));
            }
            break;
        default:
            break;
        }
    }
    if (active) {
        sendKeys(m_navigator.tick(now));
    } else {
        m_navigator.reset();
    }
}

void GamepadInput::sendKeys(const QList<Qt::Key> &keys)
{
    QWindow *window = QGuiApplication::focusWindow();
    if (!window) {
        return;
    }
    for (const Qt::Key key : keys) {
        // Microsoft's page has no key for "back"; its back button uses the
        // page's history. So on the page, B goes back in it, like a
        // browser's Back button (WebSession makes sure that never reloads
        // the page). Everywhere else, such as a dialog, it is Escape.
        if (key == Qt::Key_Escape) {
            if (QObject *page = focusedPartyPage(window)) {
                QMetaObject::invokeMethod(page, "goBackInPage");
                continue;
            }
        }
        // Enter carries its text too, as a real key press does; the page
        // listens for either.
        const QString text = key == Qt::Key_Return ? QStringLiteral("\r") : QString();
        QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, text);
        QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier, text);
        QCoreApplication::sendEvent(window, &press);
        QCoreApplication::sendEvent(window, &release);
    }
}
