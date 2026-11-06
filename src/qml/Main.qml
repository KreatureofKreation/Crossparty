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
    // While nobody can see the page (the window is in the tray, minimised,
    // or otherwise not drawn), it moves to the background holder at the
    // overlay's narrow width. Only that layout shows the "1 invite" label the
    // invite watcher reads, so this is what lets a toast appear while you are
    // in a game (docs/plan.md, Invite toasts). The page is never reloaded, so
    // the party carries on.
    readonly property bool pageInBackground: !overlayOpen && (!visible || visibility === Window.Minimized || !exposure.exposed)

    // Wayland never reports "minimised", but KDE stops drawing the window.
    WindowExposure {
        id: exposure

        window: mainWindow
    }

    onPageInBackgroundChanged: {
        if (pageInBackground) {
            partyPage.parent = backgroundHolder.pageArea;
        } else if (!overlayOpen) {
            partyPage.parent = pageHolder;
        }
    }

    // The one live page moves into the overlay and back, so the party never
    // reconnects (docs/plan.md, Overlay content).
    function openOverlay() {
        overlay.show();
        partyPage.parent = overlay.pageArea;
        partyPage.zoomFactor = overlay.pageZoom;
        overlayOpen = true;
    }

    function closeOverlay() {
        partyPage.zoomFactor = 1;
        overlayOpen = false;
        // Straight to the background holder if the window is not on screen.
        partyPage.parent = pageInBackground ? backgroundHolder.pageArea : pageHolder;
        overlay.hide();
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

            // Sized by binding rather than anchors, so it follows whichever
            // window holds it.
            PartyPage {
                id: partyPage

                width: parent ? parent.width : 0
                height: parent ? parent.height : 0
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

    BackgroundHolder {
        id: backgroundHolder

        pageWidth: overlay.pageWidth
        visible: mainWindow.pageInBackground
    }

    // While the overlay is open it shows the microphone state itself, and the
    // party is in front of the user, so the corner notices stay out of its way.
    CornerNotices {
        id: cornerNotices

        showMutedBadge: MicrophoneMute.muted && !mainWindow.overlayOpen
    }

    Connections {
        target: WebSession

        function onNotificationArrived(title, message) {
            if (!mainWindow.overlayOpen) {
                cornerNotices.presentToast(title, message);
            }
        }

        // The page gives no name, and none is shown: the toast only says that
        // an invite is waiting.
        function onInviteArrived() {
            // A minimised window can still count as active on Wayland, so
            // "on screen" is checked as well.
            const partyInFront = mainWindow.overlayOpen || (mainWindow.active && !mainWindow.pageInBackground);
            if (!partyInFront) {
                cornerNotices.presentToast(qsTr("Party invite"), qsTr("Someone has invited you to a party."));
            }
        }
    }

    // Above everything else, so the edges can be grabbed over the page too.
    WindowEdges {
        anchors.fill: parent
        window: mainWindow
    }
}
