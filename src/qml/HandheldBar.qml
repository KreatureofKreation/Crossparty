// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Crossparty

// Handheld mode's bar, in place of the top bar (docs/plan.md, Handheld mode).
// Bigger targets for thumbs, labels as well as icons, and a thick outline on
// whichever button the controller is on. Steam Input turns the D-pad into
// arrow keys and A into Enter, so that is all it needs: Left and Right move
// between buttons, Down goes to the page. There are no window buttons:
// Steam's own menu switches apps and closes them.
QQC2.ToolBar {
    id: handheldBar

    // True while one of the bar's buttons has focus.
    readonly property bool controlsFocused: muteButton.activeFocus || signOutButton.activeFocus

    signal signOutRequested
    signal pageFocusRequested

    // Moves focus to the bar, as Tab would, so the focus outline shows. A
    // controller's Menu button calls this through Main.qml.
    function focusControls() {
        (muteButton.enabled ? muteButton : signOutButton).forceActiveFocus(Qt.TabFocusReason);
    }

    implicitHeight: Kirigami.Units.gridUnit * 3
    padding: Kirigami.Units.smallSpacing

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            level: 2
            text: Qt.application.displayName
        }

        // The page keeps the arrow keys for itself, so this is how a
        // controller reaches these buttons.
        QQC2.Label {
            Layout.fillWidth: true
            text: qsTr("Menu button (≡): here and back")
            elide: Text.ElideRight
            opacity: 0.7
        }

        // Mutes the microphone itself (MicrophoneMute), and always shows
        // whether it is muted.
        HandheldButton {
            id: muteButton

            text: MicrophoneMute.muted ? qsTr("Unmute") : qsTr("Mute")
            icon.name: MicrophoneMute.muted ? "microphone-sensitivity-muted" : "audio-input-microphone"
            checkable: true
            checked: MicrophoneMute.muted
            enabled: MicrophoneMute.available
            Accessible.name: MicrophoneMute.muted ? qsTr("Unmute microphone") : qsTr("Mute microphone")
            KeyNavigation.right: signOutButton
            onClicked: MicrophoneMute.toggle()
        }

        HandheldButton {
            id: signOutButton

            text: qsTr("Sign Out and Wipe…")
            icon.name: "system-log-out"
            KeyNavigation.left: muteButton
            onClicked: handheldBar.signOutRequested()
        }
    }

    // A labelled button, at least a thumb wide, outlined thickly while the
    // controller is on it. Enter (A) presses it as well as Space.
    component HandheldButton: QQC2.Button {
        id: handheldButton

        readonly property int outlineWidth: Kirigami.Units.smallSpacing

        Layout.fillHeight: true
        Layout.minimumWidth: Kirigami.Units.gridUnit * 4
        display: QQC2.AbstractButton.TextBesideIcon
        icon.width: Kirigami.Units.iconSizes.smallMedium
        icon.height: Kirigami.Units.iconSizes.smallMedium
        focusPolicy: Qt.StrongFocus

        Keys.onReturnPressed: handheldButton.click()
        Keys.onEnterPressed: handheldButton.click()
        Keys.onDownPressed: handheldBar.pageFocusRequested()

        Rectangle {
            anchors.fill: parent
            anchors.margins: -handheldButton.outlineWidth
            visible: handheldButton.visualFocus
            color: "transparent"
            radius: Kirigami.Units.cornerRadius + handheldButton.outlineWidth
            border.width: handheldButton.outlineWidth
            border.color: Kirigami.Theme.focusColor
        }
    }
}
