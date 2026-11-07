// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include "SettingsFile.h"

#include <QObject>
#include <QQmlEngine>
#include <QSettings>
#include <QSize>

// The app's settings, as the interface sees them. They live in one local
// file in the app's own config folder; nothing is sent anywhere.
class AppSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // The window-mode size. Wayland does not let apps choose their position,
    // so only the size is kept (docs/plan.md, Window position).
    Q_PROPERTY(QSize windowSize READ windowSize WRITE setWindowSize NOTIFY windowSizeChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    QSize windowSize() const;
    void setWindowSize(const QSize &size);

Q_SIGNALS:
    void windowSizeChanged();

private:
    void save();

    QSettings m_file;
    SettingsFile::Values m_values;
    bool m_canSave;
};
