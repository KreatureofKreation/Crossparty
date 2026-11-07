// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "SettingsFile.h"

#include <QSettings>

namespace
{
// Smaller than this and Microsoft's page is unusable; larger than this is
// not a real screen. Anything outside is treated as a damaged value.
constexpr QSize smallestWindow{320, 240};
constexpr QSize largestWindow{16384, 16384};

QString versionKey()
{
    return QStringLiteral("version");
}

QString windowSizeKey()
{
    return QStringLiteral("window/size");
}
}

QSize SettingsFile::sanitisedWindowSize(const QSize &size)
{
    const bool reasonable = size.width() >= smallestWindow.width() && size.height() >= smallestWindow.height() && size.width() <= largestWindow.width()
        && size.height() <= largestWindow.height();
    return reasonable ? size : QSize();
}

SettingsFile::LoadResult SettingsFile::load(const QSettings &file)
{
    // A file with no version number is one this release wrote before saving
    // anything, or no file at all. Both read as the current version.
    bool versionIsNumber = false;
    const int version = file.value(versionKey(), currentVersion).toInt(&versionIsNumber);
    if (!versionIsNumber || version > currentVersion) {
        // Fail safe: a file we do not understand is neither read nor
        // overwritten.
        return {Values{}, false};
    }

    Values values;
    values.windowSize = sanitisedWindowSize(file.value(windowSizeKey()).toSize());
    return {values, true};
}

void SettingsFile::save(QSettings &file, const Values &values)
{
    file.setValue(versionKey(), currentVersion);
    if (values.windowSize.isValid()) {
        file.setValue(windowSizeKey(), values.windowSize);
    } else {
        file.remove(windowSizeKey());
    }
    file.sync();
}
