// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QString>

// Desktop portals answer each request on a Request object whose path is
// built from the caller's bus name and a token the caller picks. Listening
// there before making the call means the answer cannot be missed.
namespace PortalRequest
{
// For example ":1.42" and "crossparty_secret" give
// "/org/freedesktop/portal/desktop/request/1_42/crossparty_secret".
QString path(const QString &uniqueBusName, const QString &token);
}
