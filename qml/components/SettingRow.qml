import QtQuick
import QtQuick.Layouts

import "../theme"

/* Label/description plus a free-form control, used across the Settings page. */
RowLayout {
    id: settingRow

    property string label: ""
    property string description: ""
    property Component control: null

    Layout.fillWidth: true
    spacing: Theme.spaceLg

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 2

        Text {
            text: settingRow.label
            color: Theme.textPrimary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeBody
            font.weight: Font.Medium
        }

        Text {
            visible: settingRow.description.length > 0
            text: settingRow.description
            color: Theme.textSecondary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeCaption
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    Loader {
        Layout.alignment: Qt.AlignVCenter
        sourceComponent: settingRow.control
    }
}
