// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QVariantMap>

// Makes sure only one copy of the app runs, so there is never a second party
// connection. The first copy claims the app ID on the session bus. Any later
// copy asks it to show its window, then exits.
//
// It uses org.freedesktop.Application, the standard interface desktops use to
// activate apps. A Flatpak app may own its own app ID on the bus without any
// extra permission.
class SingleInstance : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Application")

public:
    explicit SingleInstance(QObject *parent = nullptr);

    // True if this is the only copy. Otherwise the running copy has been asked
    // to show itself, and this one should exit.
    bool claim();

Q_SIGNALS:
    void activationRequested();

public Q_SLOTS:
    // Called over D-Bus by a later copy, or by the desktop. The name is fixed
    // by the org.freedesktop.Application interface.
    Q_SCRIPTABLE void Activate(const QVariantMap &platformData); // NOLINT(readability-identifier-naming)
};
