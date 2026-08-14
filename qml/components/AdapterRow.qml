import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"

/* Selectable adapter summary row used by the Network page. */
Rectangle {
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
