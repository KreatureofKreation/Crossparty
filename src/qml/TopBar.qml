// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

// The window's own title bar, about 36 px tall, in place of KDE's. It follows
// the system theme and uses system icons. Mute and overlay buttons join it in
// stage 3, when those features exist.
QQC2.ToolBar {
    id: topBar

    required property Window window

    readonly property bool isMaximised: window.visibility === Window.Maximized

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
