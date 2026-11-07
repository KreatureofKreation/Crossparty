// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.layershell as LayerShell

// The muted badge and the invite and message toasts, in the top-right
// corner above games. Like the overlay it never takes the keyboard, and
// clicks pass straight through it to the game underneath, so it can never get
// in the way.
Window {
    id: cornerNotices

    property bool showMutedBadge: false
    property string toastTitle
    property string toastMessage
    readonly property bool showToast: toastTimer.running
    readonly property int edgeGap: Kirigami.Units.largeSpacing

    function presentToast(title, message) {
        toastTitle = title;
        toastMessage = message;
        toastTimer.restart();
    }

    title: qsTr("Crossparty notices")
    visible: showMutedBadge || showToast
    width: notices.implicitWidth + edgeGap * 2
    height: notices.implicitHeight + edgeGap * 2
    color: "transparent"
    flags: Qt.WindowTransparentForInput

    LayerShell.Window.scope: "crossparty-notices"
    LayerShell.Window.layer: LayerShell.Window.LayerOverlay
    LayerShell.Window.anchors: LayerShell.Window.AnchorTop | LayerShell.Window.AnchorRight
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityNone
    LayerShell.Window.activateOnShow: false

    Timer {
        id: toastTimer

        interval: 6000
    }

    Column {
        id: notices

        x: cornerNotices.edgeGap
        y: cornerNotices.edgeGap
        spacing: Kirigami.Units.smallSpacing

        Rectangle {
            anchors.right: parent.right
            visible: cornerNotices.showMutedBadge
            width: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing
            height: width
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor

            Kirigami.Icon {
                anchors.centerIn: parent
                width: Kirigami.Units.iconSizes.medium
                height: width
                source: "microphone-sensitivity-muted"
                color: Kirigami.Theme.negativeTextColor
                Accessible.name: qsTr("Microphone muted")
            }
        }

        Rectangle {
            visible: cornerNotices.showToast
            width: Kirigami.Units.gridUnit * 18
            height: toastText.implicitHeight + Kirigami.Units.largeSpacing * 2
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor

            Column {
                id: toastText

                x: Kirigami.Units.largeSpacing
                y: Kirigami.Units.largeSpacing
                width: parent.width - Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.smallSpacing

                // The title and message come from Microsoft's page, so they
                // are shown as plain text only: never as rich text or links.
                // See docs/security.md#sec-016.
                QQC2.Label {
                    width: parent.width
                    text: cornerNotices.toastTitle
                    textFormat: Text.PlainText
                    font.bold: true
                    elide: Text.ElideRight
                }

                QQC2.Label {
                    width: parent.width
                    text: cornerNotices.toastMessage
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                    visible: text.length > 0
                }

                // The user's own keys; no hint if the hotkey is not set.
                QQC2.Label {
                    width: parent.width
                    visible: GlobalShortcuts.overlayKeys.length > 0
                    text: qsTr("%1 opens the party").arg(GlobalShortcuts.overlayKeys)
                    textFormat: Text.PlainText
                    opacity: 0.7
                }
            }
        }
    }
}
