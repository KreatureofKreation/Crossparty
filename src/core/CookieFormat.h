// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QNetworkCookie>
#include <QUrl>

// Turns cookies into bytes for the encrypted file, and back again.
namespace CookieFormat
{
// Expired cookies are left out.
QByteArray toBytes(const QList<QNetworkCookie> &cookies, const QDateTime &now);

// Returns nothing if the bytes are not in our format. Lines that cannot be
// read are skipped.
QList<QNetworkCookie> fromBytes(const QByteArray &bytes);

// The cookie as it must be handed back to QtWebEngine. A host-only cookie
// (one with no leading dot in its domain) must not carry a domain, or
// Chromium treats it as a domain cookie, and refuses "__Host-" cookies
// outright. Microsoft's main sign-in cookie is one of those (docs/spike.md).
QNetworkCookie prepareForRestore(const QNetworkCookie &cookie);

// The address a cookie belongs to, which QtWebEngine needs to restore it.
QUrl originFor(const QNetworkCookie &cookie);
}
