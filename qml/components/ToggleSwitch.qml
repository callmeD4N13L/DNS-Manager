import QtQuick
import QtQuick.Controls

import "../theme"

/* App-styled on/off switch with a text label. */
Switch {
    id: root

    implicitHeight: 26
    spacing: 10

    indicator: Rectangle {
        implicitWidth: 44
        implicitHeight: 24
        radius: 12
        x: root.leftPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        color: root.checked ? Theme.accent : Theme.surfaceBorder

        Rectangle {
            x: root.checked ? parent.width - width - 3 : 3
            y: (parent.height - height) / 2
            width: 18
            height: 18
            radius: 9
            color: root.checked ? Theme.onAccent : Theme.textDisabled
            Behavior on x { NumberAnimation { duration: Theme.durationNormal } }
        }
    }

    contentItem: Text {
        text: root.text
        color: Theme.textPrimary
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontSizeBody
        verticalAlignment: Text.AlignVCenter
    }
}