// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.layershell as LayerShell

// The overlay: a compact panel in the top-right corner, on the Wayland layer
// above games. It has no content of its own; window mode hands it the one
// live party page while it is open (Main.qml), so there is still only one
// party connection.
//
// It never takes the keyboard. Games often minimise or leave full screen the
// moment they lose keyboard focus (tested with Baldur's Gate 3 and Dragon's
// Dogma 2), so the overlay is mouse-only and is opened and closed with the
// global hotkey. See docs/security.md#sec-015.
Window {
    id: overlayWindow

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
    height: Math.min(Math.round(pageHeight * pageZoom), Math.round(Screen.height * largestShareOfScreenHeight))
    color: Kirigami.Theme.backgroundColor

    LayerShell.Window.scope: "crossparty-overlay"
    LayerShell.Window.layer: LayerShell.Window.LayerOverlay
    LayerShell.Window.anchors: LayerShell.Window.AnchorTop | LayerShell.Window.AnchorRight
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityNone
    LayerShell.Window.activateOnShow: false
}
