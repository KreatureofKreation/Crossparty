// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GameMode.h"

#include "GameModeDetection.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcGameMode, "crossparty.gamemode", QtInfoMsg)

GameMode::GameMode(QObject *parent)
    : QObject(parent)
    , m_active(GameModeDetection::isGameMode(QProcessEnvironment::systemEnvironment()))
{
    if (m_active) {
        qCInfo(lcGameMode) << "Game Mode detected; using handheld mode";
    }
}

bool GameMode::isActive() const
{
    return m_active;
}
