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
    onNewWindowRequested: request => {
        if (WebSession.handleNewWindow(request)) {
            partyPage.url = request.requestedUrl;
        }
    }
    onPermissionRequested: permission => WebSession.handlePermission(permission)
}
