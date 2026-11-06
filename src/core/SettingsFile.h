// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QSize>

class QSettings;

// Reads and writes the settings file. The file carries a version number, so
// a later release can tell an old file from a new one and convert it, and an
// older release never overwrites a file written by a newer one.
namespace SettingsFile
{
constexpr int currentVersion = 1;

struct Values {
    // Invalid (QSize()) means "not saved yet": the interface uses its own
    // default.
    QSize windowSize;
};

struct LoadResult {
    Values values;
    // False when the file comes from a newer release. The app then runs with
    // defaults and leaves the file alone.
    bool canSave = true;
};

LoadResult load(const QSettings &file);
void save(QSettings &file, const Values &values);

// A window size from the file, or an invalid size if it is missing or
// unreasonable.
QSize sanitisedWindowSize(const QSize &size);
}
