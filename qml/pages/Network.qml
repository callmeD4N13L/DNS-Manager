import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../theme"

component AdapterRow: Rectangle {
    id: adapterRow

    property string adapterId: ""
    property string adapterName: ""
    property string typeLabel: ""
    property bool adapterEnabled: false
    property bool connected: false
    property bool dhcpEnabled: false
    property string ipv4: ""
    property string ipv6: ""
    property string dns: ""
    property bool selected: false

    signal select(string id)

    readonly property string statusText: !adapterEnabled ? qsTr("Disabled")
                                        : connected ? qsTr("Connected")
                                                    : qsTr("Disconnected")
    readonly property color statusColor: !adapterEnabled ? Theme.textDisabled
                                         : connected ? Theme.success
                                                     : Theme.warning

    Layout.fillWidth: true
    implicitHeight: 118
    radius: Theme.radiusMd
    color: selected ? Theme.backgroundAlt : Theme.surface
    border.color: selected ? Theme.accent : Theme.surfaceBorder
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.spaceLg
        spacing: Theme.spaceLg

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spaceSm

                Text {
                    text: adapterRow.adapterName
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeBodyLarge
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: adapterRow.statusColor
                }
            }

            Text {
                text: adapterRow.statusText + (adapterRow.typeLabel.length > 0
                                               ? " · " + adapterRow.typeLabel : "")
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeCaption
            }

            Text {
                text: qsTr("IPv4: %1   IPv6: %2").arg(
                          adapterRow.ipv4.length > 0 ? adapterRow.ipv4 : "—",
                          adapterRow.ipv6.length > 0 ? adapterRow.ipv6 : "—")
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeCaption
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                text: qsTr("DNS: %1   DHCP: %2").arg(
                          adapterRow.dns.length > 0 ? adapterRow.dns : qsTr("Automatic"),
                          adapterRow.dhcpEnabled ? qsTr("enabled") : qsTr("static"))
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeCaption
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        RadioButton {
            Layout.alignment: Qt.AlignVCenter
            checked: adapterRow.selected
            enabled: adapterRow.adapterEnabled
            onClicked: adapterRow.select(adapterRow.adapterId)
            ToolTip.visible: hovered && !adapterRow.adapterEnabled
            ToolTip.text: qsTr("This adapter is disabled")
        }
    }
}

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
