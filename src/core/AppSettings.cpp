// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "AppSettings.h"

#include <QLoggingCategory>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(lcSettings, "crossparty.settings", QtInfoMsg)

namespace
{
QString settingsFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/settings.ini");
}
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_file(settingsFilePath(), QSettings::IniFormat)
{
    const SettingsFile::LoadResult loaded = SettingsFile::load(m_file);
    m_values = loaded.values;
    m_canSave = loaded.canSave;
    if (!m_canSave) {
        qCWarning(lcSettings) << "The settings file is from a newer version of Crossparty; using defaults and leaving it unchanged";
    }
}

QSize AppSettings::windowSize() const
{
    return m_values.windowSize;
}

void AppSettings::setWindowSize(const QSize &size)
{
    const QSize sanitised = SettingsFile::sanitisedWindowSize(size);
    if (sanitised == m_values.windowSize) {
        return;
    }
    m_values.windowSize = sanitised;
    save();
    Q_EMIT windowSizeChanged();
}

void AppSettings::save()
{
    if (m_canSave) {
        SettingsFile::save(m_file, m_values);
    }
}
