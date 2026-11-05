// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Crossparty

// Window mode: the app's own top bar, with the party page filling the rest.
// The overlay borrows the same live page while it is open; handheld mode will
// do the same.
Kirigami.ApplicationWindow {
    id: mainWindow

    title: Qt.application.displayName
    // The size from last time, or a default on first start.
    width: AppSettings.windowSize.width > 0 ? AppSettings.windowSize.width : Kirigami.Units.gridUnit * 40
    height: AppSettings.windowSize.height > 0 ? AppSettings.windowSize.height : Kirigami.Units.gridUnit * 30
    // The top bar replaces KDE's title bar, so KDE draws no frame.
    flags: Qt.Window | Qt.FramelessWindowHint

    property bool overlayOpen: false

    // The one live page moves into the overlay and back, so the party never
    // reconnects (docs/plan.md, Overlay content).
    function openOverlay() {
        overlay.show();
        partyPage.parent = overlay.contentItem;
        partyPage.zoomFactor = overlay.pageZoom;
        overlayOpen = true;
    }

    function closeOverlay() {
        partyPage.parent = pageHolder;
        partyPage.zoomFactor = 1;
        overlay.hide();
        overlayOpen = false;
    }

    function toggleOverlay() {
        if (overlayOpen) {
            closeOverlay();
        } else {
            openOverlay();
        }
    }

    // Called from C++ when Crossparty is opened again (main.cpp), and from the
    // tray.
    function bringToFront() {
        show();
        raise();
        requestActivate();
    }

    // Only for a normal window: a maximised window's size is the screen's,
    // not one the user chose.
    function saveWindowSize() {
        if (visibility === Window.Windowed) {
            AppSettings.windowSize = Qt.size(width, height);
        }
    }

    // Closing sends the app to the tray, so the party keeps going. Quit is in
    // the tray menu. With no tray, closing quits, so the app is never left
    // running out of reach.
    onClosing: close => {
        saveWindowSize();
        if (TrayIcon.available) {
            close.accepted = false;
            hide();
        }
    }

    Connections {
        target: GlobalShortcuts

        function onMuteToggleRequested() {
            MicrophoneMute.toggle();
        }

        function onOverlayToggleRequested() {
            mainWindow.toggleOverlay();
        }
    }

    Connections {
        target: TrayIcon

        function onShowRequested() {
            mainWindow.bringToFront();
        }
    }

    Connections {
        target: Qt.application

        function onAboutToQuit() {
            mainWindow.saveWindowSize();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            Layout.fillWidth: true
            window: mainWindow
            overlayOpen: mainWindow.overlayOpen
            onOverlayToggleRequested: mainWindow.toggleOverlay()
        }

        Item {
            id: pageHolder

            Layout.fillWidth: true
            Layout.fillHeight: true

            PartyPage {
                id: partyPage

                anchors.fill: parent
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - Kirigami.Units.gridUnit * 4
                visible: mainWindow.overlayOpen
                icon.name: "window-duplicate"
                text: qsTr("The party is in the overlay")
                explanation: qsTr("Press Meta+Shift+X to bring it back here.")
            }
        }
    }

    OverlayWindow {
        id: overlay
    }

    // Above everything else, so the edges can be grabbed over the page too.
    WindowEdges {
        anchors.fill: parent
        window: mainWindow
    }
}
