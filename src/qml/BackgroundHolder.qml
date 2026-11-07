// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import org.kde.layershell as LayerShell

// Holds the party page while the main window is not on screen (in the tray,
// or minimised). Chromium only lays a page out in a window that is being
// drawn, and KDE stops drawing minimised windows, so the page needs somewhere
// that is always drawn: a single transparent pixel on the desktop's background
// layer, under every other window. The page is laid out at the overlay's
// narrow width inside it, so the invite watcher can read the invite count
// (docs/security.md#sec-017). Nothing of the page is ever visible, it never
// appears in the taskbar, and clicks and keys never reach it.
Window {
    id: backgroundHolder

    property int pageWidth: 440
    readonly property int pageHeight: 720
    readonly property Item pageArea: pageArea

    title: qsTr("Crossparty background")
    width: 1
    height: 1
    color: "transparent"
    flags: Qt.WindowTransparentForInput | Qt.WindowDoesNotAcceptFocus

    LayerShell.Window.scope: "crossparty-background"
    LayerShell.Window.layer: LayerShell.Window.LayerBackground
    LayerShell.Window.anchors: LayerShell.Window.AnchorTop | LayerShell.Window.AnchorLeft
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityNone
    LayerShell.Window.activateOnShow: false
    // Never pushes other windows or panels aside.
    LayerShell.Window.exclusionZone: -1

    // Larger than the window: everything outside the one pixel is cut off,
    // and the page is fully transparent to the user anyway.
    Item {
        id: pageArea

        width: backgroundHolder.pageWidth
        height: backgroundHolder.pageHeight
        opacity: 0
    }
}
