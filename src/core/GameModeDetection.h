// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QProcessEnvironment>

// Decides whether Crossparty is running inside Gamescope, the compositor
// behind Steam's Game Mode on handhelds, and so should use handheld mode.
// It reads only environment variables; nothing is asked of the system.
namespace GameModeDetection
{

// CROSSPARTY_HANDHELD=1 or =0 overrides the detection, to try handheld mode
// on the desktop or to turn it off. Otherwise Gamescope is recognised by the
// variables it sets for every app it runs.
bool isGameMode(const QProcessEnvironment &environment);

}
