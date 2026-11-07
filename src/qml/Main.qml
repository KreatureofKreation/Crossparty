// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Crossparty

// Window mode: the app's own top bar, with the party page filling the rest.
// The overlay borrows the same live page while it is open. In Game Mode the
// same window becomes handheld mode instead: full screen, with the handheld
// bar in place of the top bar (docs/plan.md, Handheld mode).
Kirigami.ApplicationWindow {
    id: mainWindow

    title: Qt.application.displayName
    // The size from last time, or a default on first start.
    width: AppSettings.windowSize.width > 0 ? AppSettings.windowSize.width : Kirigami.Units.gridUnit * 40
    height: AppSettings.windowSize.height > 0 ? AppSettings.windowSize.height : Kirigami.Units.gridUnit * 30
    // The top bar replaces KDE's title bar, so KDE draws no frame.
    flags: Qt.Window | Qt.FramelessWindowHint

    // Gamescope has no layer above games, so in handheld mode there is no
    // overlay, corner notices or background holder; Steam's own menu switches
    // between Crossparty and the game.
    readonly property bool handheld: GameMode.active
    property bool overlayOpen: false
    // While nobody can see the page (the window is in the tray, minimised,
    // or otherwise not drawn), it moves to the background holder at the
    // overlay's narrow width. Only that layout shows the "1 invite" label the
    // invite watcher reads, so this is what lets a toast appear while you are
    // in a game (docs/plan.md, Invite toasts). The page is never reloaded, so
    // the party carries on.
    readonly property bool pageInBackground: !handheld && !overlayOpen && (!visible || visibility === Window.Minimized || !exposure.exposed)

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
        if (handheld) {
            return;
        }
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
        if (handheld) {
            showFullScreen();
        } else {
            show();
        }
        raise();
        requestActivate();
    }

    // Only for a normal window: a maximised window's size is the screen's,
    // not one the user chose.
    function saveWindowSize() {
        if (!handheld && visibility === Window.Windowed) {
            AppSettings.windowSize = Qt.size(width, height);
        }
    }

    // Closing sends the app to the tray, so the party keeps going. Quit is in
    // the tray menu. With no tray, closing quits, so the app is never left
    // running out of reach. Handheld mode always quits: Game Mode has no tray.
    onClosing: close => {
        saveWindowSize();
        if (TrayIcon.available && !handheld) {
            close.accepted = false;
            hide();
        }
    }

    Component.onCompleted: {
        if (handheld) {
            showFullScreen();
            partyPage.forceActiveFocus();
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

        function onSignOutRequested() {
            mainWindow.bringToFront();
            signOutDialog.open();
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
            visible: !mainWindow.handheld
            window: mainWindow
            overlayOpen: mainWindow.overlayOpen
            onOverlayToggleRequested: mainWindow.toggleOverlay()
            onSignOutRequested: signOutDialog.open()
        }

        HandheldBar {
            Layout.fillWidth: true
            visible: mainWindow.handheld
            onSignOutRequested: signOutDialog.open()
            onPageFocusRequested: partyPage.forceActiveFocus()
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
                explanation: GlobalShortcuts.overlayKeys.length > 0 ? qsTr("Press %1 to bring it back here.").arg(GlobalShortcuts.overlayKeys) : qsTr("Press the overlay button above to bring it back here.")
            }
        }
    }

    OverlayWindow {
        id: overlay
    }

    SignOutDialog {
        id: signOutDialog
    }

    // Shown only if Sign Out and Wipe could not delete the saved sign-in, so
    // the user is never told everything was erased when it was not.
    Kirigami.PromptDialog {
        id: wipeFailedDialog

        property string filePath

        title: qsTr("Saved sign-in not deleted")
        subtitle: qsTr("You are signed out for now, but Crossparty could not delete its saved sign-in, so it would sign you back in next time. Delete this file yourself, then quit Crossparty:\n\n%1").arg(filePath)
        standardButtons: Kirigami.Dialog.Ok
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

        showMutedBadge: MicrophoneMute.muted && !mainWindow.overlayOpen && !mainWindow.handheld
    }

    Connections {
        target: WebSession

        function onNotificationArrived(title, message) {
            if (!mainWindow.overlayOpen && !mainWindow.handheld) {
                cornerNotices.presentToast(title, message);
            }
        }

        // The page gives no name, and none is shown: the toast only says that
        // an invite is waiting.
        function onWipeFailed(filePath) {
            signOutDialog.close();
            wipeFailedDialog.filePath = filePath;
            wipeFailedDialog.open();
        }

        function onInviteArrived() {
            // A minimised window can still count as active on Wayland, so
            // "on screen" is checked as well.
            const partyInFront = mainWindow.overlayOpen || (mainWindow.active && !mainWindow.pageInBackground);
            if (!partyInFront && !mainWindow.handheld) {
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
