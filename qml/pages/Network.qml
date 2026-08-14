import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../theme"

ColumnLayout {
    id: root

    spacing: Theme.spaceXl

    PageHeader {
        title: qsTr("Network")
        subtitle: qsTr("Detected adapters — select the one you want to modify")

        headerAction: Component {
            RowLayout {
                spacing: Theme.spaceMd

                SecondaryButton {
                    text: qsTr("Reset to DHCP")
                    enabled: Backend.selectedAdapterId.length > 0
                    onClicked: resetDialog.showConfirm()
                }

                SecondaryButton {
                    text: qsTr("Flush DNS Cache")
                    onClicked: Backend.flushDnsCache()
                }

                SecondaryButton {
                    text: qsTr("Refresh")
                    iconName: "refresh"
                    onClicked: Backend.refresh()
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("Adapters")
        subtitle: qsTr("%1 adapter(s) found").arg(Backend.adapters.count)

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: Math.max(listColumn.implicitHeight, height)
            clip: true

            ColumnLayout {
                id: listColumn
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: Theme.spaceMd

                Repeater {
                    model: Backend.adapters

                    AdapterRow {
                        adapterId: model.id
                        adapterName: model.friendlyName.length > 0 ? model.friendlyName : model.name
                        typeLabel: model.typeLabel
                        adapterEnabled: model.enabled
                        connected: model.connected
                        dhcpEnabled: model.dhcpEnabled
                        ipv4: model.ipv4
                        ipv6: model.ipv6
                        dns: model.dns
                        selected: model.isSelected
                        onSelect: (id) => Backend.selectAdapter(id)
                    }
                }

                Column {
                    Layout.fillWidth: true
                    Layout.topMargin: Theme.spaceXl
                    spacing: Theme.spaceSm
                    visible: Backend.adapters.count === 0

                    Icon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        name: "wifi"
                        size: 40
                        color: Theme.textDisabled
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("No adapters detected")
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeBodyLarge
                    }
                }
            }
        }
    }

    ConfirmDialog {
        id: resetDialog
        titleText: qsTr("Reset DNS to DHCP")
        message: qsTr("Reset DNS on the selected adapter to automatic (DHCP)?")
        confirmText: qsTr("Reset")
        onConfirmed: Backend.resetDns()
    }
}
