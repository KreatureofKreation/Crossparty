// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Crossparty

// The window's own title bar, about 36 px tall, in place of KDE's. It follows
// the system theme and uses system icons.
QQC2.ToolBar {
    id: topBar

    required property Window window

    readonly property bool isMaximised: window.visibility === Window.Maximized
    property bool overlayOpen: false

    signal overlayToggleRequested

    function toggleMaximised() {
        if (isMaximised) {
            window.showNormal();
        } else {
            window.showMaximized();
        }
    }

    implicitHeight: Kirigami.Units.gridUnit * 2
    padding: 0

    // Drag the bar to move the window and double-click it to maximise, the
    // same as KDE's own title bars. Moving is handed to KWin, because Wayland
    // apps cannot place their own windows.
    DragHandler {
        target: null
        onActiveChanged: {
            if (active) {
                topBar.window.startSystemMove();
            }
        }
    }

    TapHandler {
        gesturePolicy: TapHandler.DragThreshold
        onDoubleTapped: topBar.toggleMaximised()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        QQC2.Label {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            text: topBar.window.title
            elide: Text.ElideRight
        }

        TopBarButton {
            text: topBar.overlayOpen ? qsTr("Bring the party back to this window") : qsTr("Open the overlay")
            icon.name: "window-duplicate"
            checkable: true
            checked: topBar.overlayOpen
            onClicked: topBar.overlayToggleRequested()
        }

        // Mutes the microphone itself (MicrophoneMute), and always shows
        // whether it is muted.
        TopBarButton {
            text: MicrophoneMute.muted ? qsTr("Unmute microphone") : qsTr("Mute microphone")
            icon.name: MicrophoneMute.muted ? "microphone-sensitivity-muted" : "audio-input-microphone"
            checkable: true
            checked: MicrophoneMute.muted
            enabled: MicrophoneMute.available
            onClicked: MicrophoneMute.toggle()
        }

        Kirigami.Separator {
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }

        TopBarButton {
            text: qsTr("Minimise")
            icon.name: "window-minimize"
            onClicked: topBar.window.showMinimized()
        }

        TopBarButton {
            text: topBar.isMaximised ? qsTr("Restore") : qsTr("Maximise")
            icon.name: topBar.isMaximised ? "window-restore" : "window-maximize"
            onClicked: topBar.toggleMaximised()
        }

        TopBarButton {
            text: qsTr("Close")
            icon.name: "window-close"
            onClicked: topBar.window.close()
        }
    }

    // An icon-only button whose text is still read out by screen readers and
    // shown as a tooltip.
    component TopBarButton: QQC2.ToolButton {
        Layout.fillHeight: true
        Layout.preferredWidth: height
        display: QQC2.AbstractButton.IconOnly
        Accessible.name: text
        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
