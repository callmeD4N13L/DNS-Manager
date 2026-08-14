import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "."

/* Generic confirmation dialog with a themed title, message and confirm
 * button. Emits confirmed() when the user accepts. */
Dialog {
    id: root

    property string titleText: qsTr("Are you sure?")
    property string message: ""
    property string confirmText: qsTr("Confirm")
    property color confirmColor: Theme.accent

    signal confirmed()

    title: root.titleText
    modal: true
    width: 400
    padding: 0

    background: Rectangle {
        color: Theme.surface
        radius: Theme.radiusLg
        border.color: Theme.surfaceBorder
        border.width: 1
    }

    header: Item {
        implicitWidth: root.width
        implicitHeight: Theme.spaceLg * 2

        Text {
            anchors.left: parent.left
            anchors.leftMargin: Theme.spaceXl
            anchors.verticalCenter: parent.verticalCenter
            text: root.titleText
            color: Theme.textPrimary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeTitle
            font.weight: Font.DemiBold
        }
    }

    contentItem: Text {
        width: root.availableWidth
        leftPadding: Theme.spaceXl
        rightPadding: Theme.spaceXl
        topPadding: Theme.spaceLg
        bottomPadding: Theme.spaceLg
        text: root.message
        color: Theme.textPrimary
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontSizeBody
        wrapMode: Text.WordWrap
    }

    footer: Item {
        implicitWidth: root.width
        implicitHeight: 56

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.surfaceBorder
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spaceXl
            anchors.rightMargin: Theme.spaceXl
            spacing: Theme.spaceMd

            Item { Layout.fillWidth: true }

            SecondaryButton {
                text: qsTr("Cancel")
                onClicked: root.close()
            }

            PrimaryButton {
                text: root.confirmText
                accentColor: root.confirmColor
                onClicked: {
                    root.confirmed()
                    root.close()
                }
            }
        }
    }

    function showConfirm() {
        root.open()
    }
}