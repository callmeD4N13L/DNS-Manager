import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"

/* Labelled, app-styled text input used by dialogs and forms. */
ColumnLayout {
    id: root

    property string label: ""
    property string placeholder: ""
    // Holds the current text; assign to prefill, read to collect the value.
    property string fieldValue: ""
    // Optional fixed-width hint (monospace) for server addresses.
    property bool monospace: false

    spacing: Theme.spaceXs

    Text {
        visible: root.label.length > 0
        text: root.label
        color: Theme.textSecondary
        font.pixelSize: Theme.fontSizeCaption
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 38
        radius: Theme.radiusSm
        color: Theme.background
        border.color: Theme.surfaceBorder
        border.width: 1

        TextField {
            id: field
            anchors.fill: parent
            anchors.leftMargin: Theme.spaceMd
            anchors.rightMargin: Theme.spaceMd
            implicitHeight: parent.height
            verticalAlignment: TextInput.AlignVCenter
            placeholderText: root.placeholder
            placeholderColor: Theme.textDisabled
            background: Item {}
            color: Theme.textPrimary
            font.family: root.monospace ? "Consolas" : Theme.fontFamily
            font.pixelSize: Theme.fontSizeBody
            selectByMouse: true
            text: root.fieldValue
            onEditingFinished: root.fieldValue = text
        }
    }
}