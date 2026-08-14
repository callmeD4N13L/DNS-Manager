import QtQuick
import QtQuick.Controls

import "../theme"
import "."

/* Solid accent action button. */
Button {
    id: root

    property string iconName: ""
    property color accentColor: Theme.accent
    property color accentText: Theme.onAccent

    implicitHeight: 38
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true

    background: Rectangle {
        radius: Theme.radiusSm
        color: !root.enabled ? Qt.rgba(Theme.textDisabled.r, Theme.textDisabled.g, Theme.textDisabled.b, 0.35)
                             : root.down ? Qt.darker(root.accentColor, 1.15)
                             : root.hovered ? Qt.lighter(root.accentColor, 1.06)
                                            : root.accentColor
    }

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: Theme.spaceSm

            Icon {
                id: icon
                anchors.verticalCenter: parent.verticalCenter
                name: root.iconName
                size: 16
                color: root.accentText
                visible: root.iconName.length > 0
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.text
                color: root.accentText
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeBody
                font.weight: Font.DemiBold
            }
        }
    }
}
