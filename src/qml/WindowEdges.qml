// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

import QtQuick
import org.kde.kirigami as Kirigami

// Without KDE's frame, the window needs its own resize edges. Each edge hands
// the resize to KWin, as KDE's frame would.
Item {
    id: windowEdges

    required property Window window

    readonly property int thickness: Kirigami.Units.smallSpacing

    visible: window.visibility === Window.Windowed

    WindowEdge {
        edges: Qt.LeftEdge
        cursor: Qt.SizeHorCursor
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: windowEdges.thickness
    }

    WindowEdge {
        edges: Qt.RightEdge
        cursor: Qt.SizeHorCursor
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        width: windowEdges.thickness
    }

    WindowEdge {
        edges: Qt.TopEdge
        cursor: Qt.SizeVerCursor
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: windowEdges.thickness
    }

    WindowEdge {
        edges: Qt.BottomEdge
        cursor: Qt.SizeVerCursor
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: windowEdges.thickness
    }

    WindowEdge {
        edges: Qt.TopEdge | Qt.LeftEdge
        cursor: Qt.SizeFDiagCursor
        anchors {
            left: parent.left
            top: parent.top
        }
        width: windowEdges.thickness * 3
        height: windowEdges.thickness * 3
    }

    WindowEdge {
        edges: Qt.TopEdge | Qt.RightEdge
        cursor: Qt.SizeBDiagCursor
        anchors {
            right: parent.right
            top: parent.top
        }
        width: windowEdges.thickness * 3
        height: windowEdges.thickness * 3
    }

    WindowEdge {
        edges: Qt.BottomEdge | Qt.LeftEdge
        cursor: Qt.SizeBDiagCursor
        anchors {
            left: parent.left
            bottom: parent.bottom
        }
        width: windowEdges.thickness * 3
        height: windowEdges.thickness * 3
    }

    WindowEdge {
        edges: Qt.BottomEdge | Qt.RightEdge
        cursor: Qt.SizeFDiagCursor
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        width: windowEdges.thickness * 3
        height: windowEdges.thickness * 3
    }

    component WindowEdge: Item {
        id: windowEdge

        required property int edges
        required property int cursor

        HoverHandler {
            cursorShape: windowEdge.cursor
        }

        DragHandler {
            target: null
            onActiveChanged: {
                if (active) {
                    windowEdges.window.startSystemResize(windowEdge.edges);
                }
            }
        }
    }
}
