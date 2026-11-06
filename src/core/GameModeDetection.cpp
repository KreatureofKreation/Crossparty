// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "GameModeDetection.h"

#include <QStringList>

bool GameModeDetection::isGameMode(const QProcessEnvironment &environment)
{
    const QString override = environment.value(QStringLiteral("CROSSPARTY_HANDHELD"));
    if (override == QLatin1String("1")) {
        return true;
    }
    if (override == QLatin1String("0")) {
        return false;
    }

    // Gamescope names itself as the desktop. The variable can list several
    // names, separated by colons.
    const QStringList desktops = environment.value(QStringLiteral("XDG_CURRENT_DESKTOP")).split(QLatin1Char(':'), Qt::SkipEmptyParts);
    for (const QString &desktop : desktops) {
        if (desktop.compare(QLatin1String("gamescope"), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    // Set by Gamescope even when the desktop name is changed.
    return !environment.value(QStringLiteral("GAMESCOPE_WAYLAND_DISPLAY")).isEmpty();
}
