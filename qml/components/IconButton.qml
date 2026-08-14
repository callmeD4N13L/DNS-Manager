import QtQuick
import QtQuick.Controls

import "../theme"

/* Compact square icon button (favorite, edit, delete, ...). */
Button {
    id: root

    property string iconName: ""
    property color iconColor: Theme.textSecondary
    property int iconSize: 16

    implicitWidth: 30
    implicitHeight: 30
    hoverEnabled: true

    background: Rectangle {
        radius: Theme.radiusSm
        color: root.hovered && root.enabled ? Theme.surfaceHover : "transparent"
    }

    contentItem: Icon {
        anchors.centerIn: parent
        name: root.iconName
        size: root.iconSize
        color: root.enabled ? root.iconColor : Theme.textDisabled
    }
}
