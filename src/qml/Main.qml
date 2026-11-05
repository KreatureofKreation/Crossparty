// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import QtWebEngine
import org.kde.kirigami as Kirigami
import Crossparty

// Window mode: the app's own top bar, with Microsoft's page filling the rest.
// Every decision about what the page may do is made by WebSession in the C++
// core, never here.
Kirigami.ApplicationWindow {
    id: mainWindow

    title: Qt.application.displayName
    width: Kirigami.Units.gridUnit * 40
    height: Kirigami.Units.gridUnit * 30
    // The top bar replaces KDE's title bar, so KDE draws no frame.
    flags: Qt.Window | Qt.FramelessWindowHint

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            Layout.fillWidth: true
            window: mainWindow
        }

        WebEngineView {
            id: webView

            Layout.fillWidth: true
            Layout.fillHeight: true
            profile: WebSession.profile
            // Waits for the saved sign-in to be restored first.
            url: WebSession.ready ? WebSession.startUrl : ""

            onNavigationRequested: request => WebSession.handleNavigation(request)
            onNewWindowRequested: request => {
                if (WebSession.handleNewWindow(request)) {
                    webView.url = request.requestedUrl;
                }
            }
            onPermissionRequested: permission => WebSession.handlePermission(permission)
        }
    }

    // Above everything else, so the edges can be grabbed over the page too.
    WindowEdges {
        anchors.fill: parent
        window: mainWindow
    }
}
