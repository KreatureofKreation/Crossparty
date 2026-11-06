// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "SiteAllowlist.h"

#include <QUrl>

namespace
{
constexpr int standardHttpsPort = 443;
}

QStringList SiteAllowlist::allowedHosts()
{
    // Taken from where the stage 1 spike actually went while signing in and
    // joining a party (docs/spike.md). Add a host only after seeing the page
    // need it, and note why here.
    return {
        // The app itself: xbox.com/play, including party chat.
        QStringLiteral("www.xbox.com"),
        // Microsoft account sign-in.
        QStringLiteral("login.live.com"),
        // Reached through a redirect during sign-in.
        QStringLiteral("login.microsoftonline.com"),
        // Microsoft account security checks that can interrupt sign-in, such
        // as verifying a recovery email. Without it, sign-in could not finish
        // inside the app.
        QStringLiteral("account.live.com"),
        // Microsoft's newer Xbox web app, and the app's start page. Sign-in
        // can return here; without it, picking an account sent the user out
        // to their browser, signed out.
        QStringLiteral("play.xbox.com"),
    };
}

QUrl SiteAllowlist::startUrl()
{
    // The older www.xbox.com/play page has no invite count for the invite
    // watcher to read (SEC-017), and sign-in already sends people here.
    return QUrl(QStringLiteral("https://play.xbox.com/"));
}

bool SiteAllowlist::isAllowedInApp(const QUrl &url)
{
    if (!url.isValid() || url.scheme() != QLatin1String("https")) {
        return false;
    }

    // An address such as "https://www.xbox.com@attacker.example" really goes
    // to attacker.example. No Microsoft page needs a user name in the address,
    // so refuse any that has one.
    if (!url.userInfo().isEmpty()) {
        return false;
    }

    const int port = url.port();
    if (port != -1 && port != standardHttpsPort) {
        return false;
    }

    // QUrl stores host names in lower case, so "WWW.XBOX.COM" matches too.
    return allowedHosts().contains(url.host());
}
