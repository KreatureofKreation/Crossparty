// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.layershell as LayerShell
import Crossparty

// The overlay: a compact panel in the top-right corner, on the Wayland layer
// above games. Window mode hands it the one live party page while it is open
// (Main.qml), so there is still only one party connection.
//
// It never takes the keyboard. Games often minimise or leave full screen the
// moment they lose keyboard focus (tested with Baldur's Gate 3 and Dragon's
// Dogma 2), so the overlay is mouse-only and is opened and closed with the
// global hotkey. See docs/security.md#sec-015.
Window {
    id: overlayWindow

    // Where the party page goes while the overlay is open.
    readonly property Item pageArea: pageArea

    // The page is shown slightly zoomed out, and narrow enough in its own
    // units (CSS pixels) that xbox.com switches to the layout where the party
    // panel fills the width. Both are in logical pixels, so the fit is the same
    // at any resolution or screen scaling, and the user's font size cannot
    // change it. Tuned by hand on 2026-11-05.
    readonly property real pageZoom: 0.9
    readonly property int pageWidth: 440
    readonly property int pageHeight: 720
    // On small screens the panel shrinks to leave a strip of the game showing.
    readonly property real largestShareOfScreenHeight: 0.9

    title: qsTr("Crossparty overlay")
    width: Math.round(pageWidth * pageZoom)
    height: Math.min(Math.round(pageHeight * pageZoom) + statusBar.implicitHeight, Math.round(Screen.height * largestShareOfScreenHeight))
    color: Kirigami.Theme.backgroundColor

    LayerShell.Window.scope: "crossparty-overlay"
    LayerShell.Window.layer: LayerShell.Window.LayerOverlay
    LayerShell.Window.anchors: LayerShell.Window.AnchorTop | LayerShell.Window.AnchorRight
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityNone
    LayerShell.Window.activateOnShow: false

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            id: pageArea

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // The muted badge sits where the overlay is, so while the overlay is
        // open the microphone state is shown here instead.
        QQC2.ToolBar {
            id: statusBar

            Layout.fillWidth: true
            position: QQC2.ToolBar.Footer

            RowLayout {
                anchors.fill: parent

                Kirigami.Icon {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    source: MicrophoneMute.muted ? "microphone-sensitivity-muted" : "audio-input-microphone"
                    color: MicrophoneMute.muted ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: !MicrophoneMute.available ? qsTr("Mute unavailable") : MicrophoneMute.muted ? qsTr("Microphone muted") : qsTr("Microphone on")
                    elide: Text.ElideRight
                }

                // The user's own keys, which can differ from the defaults.
                QQC2.Label {
                    visible: text.length > 0 && MicrophoneMute.available
                    text: GlobalShortcuts.muteKeys
                    textFormat: Text.PlainText
                    opacity: 0.7
                }
            }
        }
    }
}
