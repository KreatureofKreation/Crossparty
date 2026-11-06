// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtWebEngine
import Crossparty

// Microsoft's party chat page. Every mode (window now; overlay and handheld
// later) shows this same page, so there is only ever one party connection.
// Every decision about what the page may do is made by WebSession in the C++
// core, never here.
WebEngineView {
    id: partyPage

    profile: WebSession.profile
    // Waits for the saved sign-in to be restored first.
    url: WebSession.ready ? WebSession.startUrl : ""

    onNavigationRequested: request => WebSession.handleNavigation(request)
    // Never loaded here: that would replace the party page.
    onNewWindowRequested: request => WebSession.handleNewWindow(request)
    onPermissionRequested: permission => WebSession.handlePermission(permission)
    onJavaScriptConsoleMessage: (level, message, lineNumber, sourceId) => WebSession.handleConsoleMessage(message, partyPage.url)

    // The read-only invite watcher (docs/security.md#sec-017). Its source is
    // in the C++ core, not here.
    Component.onCompleted: userScripts.insert(WebSession.inviteWatchScript)
}
