// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Crossparty

// Window mode: the app's own top bar, with the party page filling the rest.
// The overlay and handheld modes will be their own views around the same
// PartyPage and the same core objects.
Kirigami.ApplicationWindow {
    id: mainWindow

    title: Qt.application.displayName
    // The size from last time, or a default on first start.
    width: AppSettings.windowSize.width > 0 ? AppSettings.windowSize.width : Kirigami.Units.gridUnit * 40
    height: AppSettings.windowSize.height > 0 ? AppSettings.windowSize.height : Kirigami.Units.gridUnit * 30
    // The top bar replaces KDE's title bar, so KDE draws no frame.
    flags: Qt.Window | Qt.FramelessWindowHint

    // Saved on close, and only for a normal window: a maximised window's size
    // is the screen's, not one the user chose.
    onClosing: {
        if (visibility === Window.Windowed) {
            AppSettings.windowSize = Qt.size(width, height);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            Layout.fillWidth: true
            window: mainWindow
        }

        PartyPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    // Above everything else, so the edges can be grabbed over the page too.
    WindowEdges {
        anchors.fill: parent
        window: mainWindow
    }
}
