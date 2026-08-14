import QtQuick
import QtQuick.Layouts

import "../theme"

/* Generic surface container with optional header (title / subtitle / action).
 * Children become the card body: Card { Rectangle { ... } } */
Rectangle {
    id: root

    default property alias content: contentArea.data

    property string title: ""
    property string subtitle: ""
    property Component headerAction: null
    property bool showHeader: title.length > 0 || subtitle.length > 0 || headerAction !== null

    color: Theme.surface
    radius: Theme.radiusMd
    border.color: Theme.surfaceBorder
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spaceLg
        spacing: Theme.spaceMd

        RowLayout {
            Layout.fillWidth: true
            visible: root.showHeader
            spacing: Theme.spaceMd

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 2

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTitle
                    font.weight: Font.DemiBold
                }

                Text {
                    visible: root.subtitle.length > 0
                    text: root.subtitle
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeCaption
                }
            }

            Loader {
                Layout.alignment: Qt.AlignVCenter
                sourceComponent: root.headerAction
            }
        }

        Item {
            id: contentArea
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
