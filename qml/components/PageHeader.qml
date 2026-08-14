import QtQuick
import QtQuick.Layouts

import "../theme"

/* Page header: title, optional subtitle, optional trailing action. */
ColumnLayout {
    id: root

    property string title: ""
    property string subtitle: ""
    property Component headerAction: null

    spacing: 4

    RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spaceMd

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                text: root.title
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeHeading
                font.weight: Font.DemiBold
            }

            Text {
                visible: root.subtitle.length > 0
                text: root.subtitle
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeBody
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }

        Loader {
            Layout.alignment: Qt.AlignVCenter
            sourceComponent: root.headerAction
        }
    }
}
