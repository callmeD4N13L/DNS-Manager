import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "."

/* A saved DNS profile row: name, servers, favorite + actions. */
Rectangle {
    id: root

    property string profileName: ""
    property string provider: ""
    property string primaryIpv4: ""
    property string secondaryIpv4: ""
    property string primaryIpv6: ""
    property string secondaryIpv6: ""
    property bool isActive: false
    property bool isFavorite: false

    signal applyClicked()
    signal editClicked()
    signal deleteClicked()
    signal toggleFavorite()

    readonly property string dnsLine: [
        root.primaryIpv4,
        root.secondaryIpv4,
        root.primaryIpv6,
        root.secondaryIpv6
    ].filter(function (s) { return s.length > 0; }).join(" / ")

    color: root.isActive ? Theme.backgroundAlt : Theme.surface
    radius: Theme.radiusMd
    border.color: root.isActive ? Theme.accent : Theme.surfaceBorder
    border.width: 1

    implicitHeight: 74

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spaceLg
        anchors.rightMargin: Theme.spaceMd
        anchors.topMargin: Theme.spaceMd
        anchors.bottomMargin: Theme.spaceMd
        spacing: Theme.spaceMd

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 2

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spaceSm

                Text {
                    text: root.profileName
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeBodyLarge
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Icon {
                    name: "check"
                    size: 14
                    color: Theme.success
                    visible: root.isActive
                }
            }

            Text {
                visible: root.provider.length > 0
                text: root.provider
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeCaption
            }

            Text {
                text: root.dnsLine
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeCaption
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        IconButton {
            Layout.alignment: Qt.AlignVCenter
            iconName: "star"
            iconColor: root.isFavorite ? Theme.warning : Theme.textDisabled
            onClicked: root.toggleFavorite()
            ToolTip.visible: hovered
            ToolTip.text: root.isFavorite ? qsTr("Remove from favorites") : qsTr("Add to favorites")
        }

        IconButton {
            Layout.alignment: Qt.AlignVCenter
            iconName: "edit"
            onClicked: root.editClicked()
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Edit profile")
        }

        IconButton {
            Layout.alignment: Qt.AlignVCenter
            iconName: "trash"
            onClicked: root.deleteClicked()
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Delete profile")
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Apply")
            onClicked: root.applyClicked()
        }
    }
}
