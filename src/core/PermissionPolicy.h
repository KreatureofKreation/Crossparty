// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QWebEnginePermission>

class QUrl;

// Decides which browser permissions a page gets. Deny by default; see
// docs/security.md#sec-011.
namespace PermissionPolicy
{
bool isAllowed(QWebEnginePermission::PermissionType type, const QUrl &origin);
}
