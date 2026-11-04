// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import org.kde.kirigami as Kirigami

// Stage 0 placeholder: an empty window that follows the system theme. The
// window, overlay and handheld views replace it in stage 2.
Kirigami.ApplicationWindow {
    id: mainWindow

    title: Qt.application.displayName
    width: Kirigami.Units.gridUnit * 40
    height: Kirigami.Units.gridUnit * 30
}
