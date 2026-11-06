// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "NavigationPolicy.h"

#include "SiteAllowlist.h"

#include <QUrl>

NavigationPolicy::Decision NavigationPolicy::decideForPage(const QUrl &url)
{
    if (SiteAllowlist::isAllowedInApp(url)) {
        return Decision::LoadInApp;
    }

    // A normal web link, for example one pasted into party chat. The user's
    // own browser is the right place for it.
    if (url.isValid() && url.scheme() == QLatin1String("https")) {
        return Decision::OpenInBrowser;
    }

    // Anything else (plain http, file:, data:, javascript:, or another app's
    // link scheme) is refused. Handing those to the desktop could open local
    // files or start other programs.
    return Decision::Block;
}

NavigationPolicy::Decision NavigationPolicy::decideForEmbeddedFrame(const QUrl &url)
{
    Q_UNUSED(url)
    // Microsoft's pages embed frames from other hosts (for example for
    // sign-in checks), and a frame cannot be moved to the browser on its own.
    // Frames therefore load, but they can never replace the main page, and
    // they get no permissions unless their own origin is allowed
    // (PermissionPolicy).
    return Decision::LoadInApp;
}
