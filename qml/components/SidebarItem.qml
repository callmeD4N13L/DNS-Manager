import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "."

/* Single navigation entry. */
Item {
    id: root

    property string text: ""
    property string iconName: ""
    property bool selected: false

    signal clicked()

    implicitHeight: 40

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: Theme.radiusSm
        color: root.selected ? Theme.surface
                             : hoverHandler.hovered && !pressedHandler.pressed ? Theme.surfaceHover
                                                                               : "transparent"
    }

    Rectangle {
        visible: root.selected
        width: 3
        height: parent.height * 0.45
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        color: Theme.accent
        radius: 2
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spaceLg
        anchors.rightMargin: Theme.spaceMd
        spacing: Theme.spaceMd

        Icon {
            Layout.alignment: Qt.AlignVCenter
            name: root.iconName
            size: 18
            color: root.selected ? Theme.accent : Theme.textSecondary
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            text: root.text
            color: root.selected ? Theme.textPrimary : Theme.textSecondary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeBody
            font.weight: root.selected ? Font.DemiBold : Font.Medium
            elide: Text.ElideRight
        }
    }

    HoverHandler {
        id: hoverHandler
    }

    TapHandler {
        id: pressedHandler
        onTapped: root.clicked()
    }
}
