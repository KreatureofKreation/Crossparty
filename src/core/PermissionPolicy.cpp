// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "PermissionPolicy.h"

#include "SiteAllowlist.h"

#include <QUrl>

namespace
{
// Party chat needs the microphone, and invite alerts arrive as browser
// notifications. Playing sound needs no permission at all. Everything else
// (camera, screen sharing, location, clipboard, and so on) is refused.
bool isNeededByPartyChat(QWebEnginePermission::PermissionType type)
{
    switch (type) {
    case QWebEnginePermission::PermissionType::MediaAudioCapture:
    case QWebEnginePermission::PermissionType::Notifications:
        return true;
    default:
        return false;
    }
}
}

bool PermissionPolicy::isAllowed(QWebEnginePermission::PermissionType type, const QUrl &origin)
{
    return isNeededByPartyChat(type) && SiteAllowlist::isAllowedInApp(origin);
}
