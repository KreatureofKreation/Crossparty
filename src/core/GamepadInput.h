// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include "GamepadNavigator.h"

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

// Lets a controller drive Crossparty without any Steam Input setup. It reads
// controllers with SDL3 and sends the keys GamepadNavigator picks to whatever
// has keyboard focus in Crossparty: the handheld bar, a dialog, or
// Microsoft's page.
//
// Only while Crossparty is the active app. When it is not, controllers are
// not read at all, and anything pressed meanwhile is thrown away when it
// becomes active again, so nothing pressed in a game can ever reach the
// party. Microsoft's page never sees a controller: the app has no /run/udev,
// which Chromium needs to find one. See docs/security.md#sec-024.
class GamepadInput : public QObject
{
    Q_OBJECT

public:
    explicit GamepadInput(QObject *parent = nullptr);
    ~GamepadInput() override;

private:
    void onApplicationStateChanged(Qt::ApplicationState state);
    void poll();
    void sendKeys(const QList<Qt::Key> &keys);

    bool m_started = false;
    QTimer m_pollTimer;
    QElapsedTimer m_clock;
    GamepadNavigator m_navigator;
};
