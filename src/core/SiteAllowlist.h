// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QStringList>

class QUrl;

// The only sites that load inside the app. Everything else is either opened in
// the user's default browser or blocked. See docs/security.md#sec-010.
namespace SiteAllowlist
{
// Exact host names, never patterns, so a look-alike such as
// "www.xbox.com.example.net" can never match.
QStringList allowedHosts();

// True only for https addresses on an allowed host, on the standard port, with
// no user name or password in the address.
bool isAllowedInApp(const QUrl &url);
}
