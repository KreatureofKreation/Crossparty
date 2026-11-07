// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QString>
#include <QtTypes>

// Rules for which recording streams the mute controls. See
// docs/security.md#sec-014.
namespace MicrophoneStreams
{
// The property PipeWire itself sets on every stream from a Flatpak app, from
// the sandbox's own records. An app cannot set or change it.
constexpr char appIdProperty[] = "pipewire.access.portal.app_id";

// True only for streams PipeWire says belong to this app. A stream without
// the property (for example when the app runs outside Flatpak) is never
// touched.
bool isOurs(const QString &streamAppId, const QString &ourAppId);

enum class MuteAction : quint8 {
    None,
    // A new stream: give it the mute state the user chose.
    ApplyChoice,
    // A known stream changed by someone else, for example the user in
    // Plasma's volume applet: follow it.
    AdoptStreamState,
};

MuteAction decide(bool isNewStream, bool streamIsMuted, bool userWantsMuted);
}
