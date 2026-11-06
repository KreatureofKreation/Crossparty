// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QQmlEngine>

// Tells the interface whether to use handheld mode. Decided once at start:
// Game Mode does not change while an app is running.
class GameMode : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool active READ isActive CONSTANT)

public:
    explicit GameMode(QObject *parent = nullptr);

    bool isActive() const;

private:
    bool m_active;
};
