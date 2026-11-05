// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtWebEngine
import org.kde.kirigami as Kirigami
import Crossparty

// Window mode: Microsoft's page fills the window. The top bar arrives later in
// stage 2. Every decision about what the page may do is made by WebSession in
// the C++ core, never here.
Kirigami.ApplicationWindow {
    id: mainWindow

    title: Qt.application.displayName
    width: Kirigami.Units.gridUnit * 40
    height: Kirigami.Units.gridUnit * 30

    WebEngineView {
        id: webView

        anchors.fill: parent
        profile: WebSession.profile
        url: WebSession.startUrl

        onNavigationRequested: request => WebSession.handleNavigation(request)
        onNewWindowRequested: request => {
            if (WebSession.handleNewWindow(request)) {
                webView.url = request.requestedUrl;
            }
        }
        onPermissionRequested: permission => WebSession.handlePermission(permission)
    }
}
