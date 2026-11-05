// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QtTypes>

class QUrl;

// Decides what happens when the page tries to go somewhere: a link click, a
// redirect, or a request for a new window. See docs/security.md#sec-010.
namespace NavigationPolicy
{
enum class Decision : quint8 {
    LoadInApp,
    OpenInBrowser,
    Block,
};

// For the main page and for requests to open a new window.
Decision decideForPage(const QUrl &url);

// For frames embedded inside an allowed page.
Decision decideForEmbeddedFrame(const QUrl &url);
}
