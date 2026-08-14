import QtQuick
import QtQuick.Layouts

import "../theme"
import "."

/* Compact status card: colored dot + label + value, optional action button. */
Rectangle {
    id: root

    property string label: ""
    property string value: ""
    property color valueColor: Theme.textPrimary
    property color dotColor: Theme.success
    property bool showDot: true
    property string actionText: ""
    property bool actionEnabled: true

    signal actionClicked()

    color: Theme.surface
    radius: Theme.radiusMd
    border.color: Theme.surfaceBorder
    border.width: 1

    implicitHeight: 84

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.spaceLg
        spacing: Theme.spaceMd

        Rectangle {
            visible: root.showDot
            width: 8
            height: 8
            radius: 4
            color: root.dotColor
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                text: root.label
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeCaption
            }

            Text {
                text: root.value
                color: root.valueColor
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeBodyLarge
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        SecondaryButton {
            visible: root.actionText.length > 0
            enabled: root.actionEnabled
            text: root.actionText
            implicitHeight: 30
            onClicked: root.actionClicked()
        }
    }
}
