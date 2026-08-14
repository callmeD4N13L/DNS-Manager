import QtQuick
import QtQuick.Controls

import "../theme"
import "."

/* Outlined / neutral button. */
Button {
    id: root

    property string iconName: ""

    implicitHeight: 38
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true

    background: Rectangle {
        radius: Theme.radiusSm
        color: root.down ? Theme.surfaceHover
                         : root.hovered ? Theme.surfaceHover
                                        : "transparent"
        border.color: !root.enabled ? Theme.textDisabled : Theme.surfaceBorder
        border.width: 1
    }

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: Theme.spaceSm

            Icon {
                anchors.verticalCenter: parent.verticalCenter
                name: root.iconName
                size: 16
                color: root.enabled ? Theme.textSecondary : Theme.textDisabled
                visible: root.iconName.length > 0
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.text
                color: root.enabled ? Theme.textPrimary : Theme.textDisabled
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeBody
                font.weight: Font.Medium
            }
        }
    }
}
