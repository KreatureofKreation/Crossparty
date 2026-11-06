// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "MicrophoneStreams.h"

bool MicrophoneStreams::isOurs(const QString &streamAppId, const QString &ourAppId)
{
    return !ourAppId.isEmpty() && streamAppId == ourAppId;
}

MicrophoneStreams::MuteAction MicrophoneStreams::decide(bool isNewStream, bool streamIsMuted, bool userWantsMuted)
{
    if (streamIsMuted == userWantsMuted) {
        return MuteAction::None;
    }
    // Muting before joining a party has to hold when the page opens the
    // microphone, so a new stream always gets the user's choice.
    return isNewStream ? MuteAction::ApplyChoice : MuteAction::AdoptStreamState;
}
