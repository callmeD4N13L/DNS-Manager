import QtQuick
import QtQuick.Layouts

import "../components"
import "../theme"

ColumnLayout {
    id: root

    property int activePage: 0

    spacing: Theme.spaceXl

    PageHeader {
        title: qsTr("Dashboard")
        subtitle: qsTr("Overview of the active adapter and its DNS configuration")
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spaceXl

        // --- Current DNS -------------------------------------------------
        Card {
            id: dnsCard
            Layout.fillHeight: true
            Layout.preferredWidth: 380
            title: qsTr("Current DNS")
            subtitle: Backend.currentDnsAdapterName.length > 0
                      ? Backend.currentDnsAdapterName
                      : qsTr("No adapter selected")

            ColumnLayout {
                spacing: Theme.spaceSm

                Text {
                    text: qsTr("IPv4")
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeCaption
                }
                Text {
                    text: Backend.currentDnsPrimaryIpv4.length > 0
                          ? Backend.currentDnsPrimaryIpv4
                          : qsTr("Not set")
                    color: Backend.currentDnsPrimaryIpv4.length > 0
                           ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: Theme.fontSizeBodyLarge
                    font.weight: Font.DemiBold
                }
                Text {
                    text: qsTr("Secondary")
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeCaption
                }
                Text {
                    text: Backend.currentDnsSecondaryIpv4.length > 0
                          ? Backend.currentDnsSecondaryIpv4
                          : qsTr("Not set")
                    color: Backend.currentDnsSecondaryIpv4.length > 0
                           ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: Theme.fontSizeBody
                    elide: Text.ElideRight
                }

                Text {
                    text: qsTr("IPv6")
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeCaption
                }
                Text {
                    text: Backend.currentDnsPrimaryIpv6.length > 0
                          ? Backend.currentDnsPrimaryIpv6
                          : qsTr("Not set")
                    color: Backend.currentDnsPrimaryIpv6.length > 0
                           ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: Theme.fontSizeBodyLarge
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                Text {
                    text: qsTr("Secondary")
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeCaption
                }
                Text {
                    text: Backend.currentDnsSecondaryIpv6.length > 0
                          ? Backend.currentDnsSecondaryIpv6
                          : qsTr("Not set")
                    color: Backend.currentDnsSecondaryIpv6.length > 0
                           ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: Theme.fontSizeBody
                    elide: Text.ElideRight
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spaceMd

                    PrimaryButton {
                        text: qsTr("Apply")
                        enabled: Backend.activeProfileId.length > 0 && Backend.currentDnsAvailable
                        onClicked: Backend.settings.confirmBeforeApply
                                   ? applyConfirmDialog.showConfirm() : Backend.applyActive()
                    }
                    SecondaryButton {
                        text: qsTr("Reset to DHCP")
                        enabled: Backend.currentDnsAvailable
                        onClicked: resetDialog.showConfirm()
                    }
                }
            }
        }

        // --- Status column ----------------------------------------------
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: Theme.spaceLg

            StatusCard {
                Layout.fillWidth: true
                label: qsTr("Connection status")
                value: Backend.currentAdapterConnected ? qsTr("Connected") : qsTr("Disconnected")
                dotColor: Backend.currentAdapterConnected ? Theme.success : Theme.error
            }

            StatusCard {
                Layout.fillWidth: true
                label: qsTr("Current profile")
                value: Backend.activeProfileName.length > 0
                       ? Backend.activeProfileName : qsTr("None")
                dotColor: Backend.activeProfileName.length > 0
                          ? Theme.accent : Theme.textSecondary
            }

            StatusCard {
                Layout.fillWidth: true
                label: qsTr("DNS latency")
                value: Backend.dnsLatencyMs >= 0
                       ? qsTr("%1 ms").arg(Backend.dnsLatencyMs) : qsTr("—")
                dotColor: Backend.dnsLatencyMs < 0 ? Theme.textSecondary
                          : Backend.dnsLatencyMs < 100 ? Theme.success
                          : Backend.dnsLatencyMs < 300 ? Theme.warning : Theme.error
                actionText: qsTr("Test")
                actionEnabled: Backend.currentDnsAvailable
                onActionClicked: Backend.testCurrentDns()
            }

            StatusCard {
                Layout.fillWidth: true
                label: qsTr("DHCP")
                value: Backend.currentDnsIsDhcp ? qsTr("Enabled") : qsTr("Disabled")
                dotColor: Backend.currentDnsIsDhcp ? Theme.success : Theme.textSecondary
            }

            Item { Layout.fillHeight: true }
        }
    }

    // --- Saved profiles ---------------------------------------------------
    Card {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("Saved DNS Profiles")
        subtitle: qsTr("One-click switch between your saved configurations")

        ColumnLayout {
            spacing: Theme.spaceSm

            Repeater {
                model: Backend.profiles

                DnsCard {
                    Layout.fillWidth: true
                    profileName: model.name
                    provider: model.provider
                    primaryIpv4: model.primaryIpv4
                    secondaryIpv4: model.secondaryIpv4
                    primaryIpv6: model.primaryIpv6
                    secondaryIpv6: model.secondaryIpv6
                    isFavorite: model.favorite
                    isActive: model.id === Backend.activeProfileId

                    onToggleFavorite: Backend.profiles.toggleFavorite(model.id)
                    onApplyClicked: Backend.applyProfile(model.id)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: Theme.spaceSm

                Item { Layout.fillWidth: true }

                SecondaryButton {
                    text: qsTr("Add DNS Profile")
                    iconName: "plus"
                    onClicked: profileDialog.openForAdd()
                }
            }
        }
    }

    ProfileDialog {
        id: profileDialog
    }

    ConfirmDialog {
        id: resetDialog
        titleText: qsTr("Reset DNS to DHCP")
        message: qsTr("Reset DNS on “%1” to automatic (DHCP)?")
            .arg(Backend.currentDnsAdapterName)
        confirmText: qsTr("Reset")
        onConfirmed: Backend.resetDns()
    }

    ConfirmDialog {
        id: applyConfirmDialog
        titleText: qsTr("Apply profile")
        message: qsTr("Apply “%1” to “%2”?")
            .arg(Backend.activeProfileName, Backend.currentDnsAdapterName)
        confirmText: qsTr("Apply")
        onConfirmed: Backend.applyActive()
    }
}
