import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../theme"

ColumnLayout {
    id: root

    spacing: Theme.spaceXl

    PageHeader {
        title: qsTr("DNS Profiles")
        subtitle: qsTr("Create, edit and switch between saved DNS configurations")

        headerAction: Component {
            PrimaryButton {
                text: qsTr("Add Profile")
                iconName: "plus"
                onClicked: profileDialog.openForAdd()
            }
        }
    }

    Card {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("All profiles")
        subtitle: qsTr("%1 profiles").arg(Backend.profiles.count)

        ColumnLayout {
            spacing: Theme.spaceMd

            // --- Search ------------------------------------------------
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 38
                radius: Theme.radiusSm
                color: Theme.background
                border.color: Theme.surfaceBorder
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.spaceMd
                    anchors.rightMargin: Theme.spaceSm
                    spacing: Theme.spaceSm

                    Icon {
                        name: "search"
                        size: 16
                        color: Theme.textSecondary
                    }

                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        implicitHeight: parent.height
                        placeholderText: qsTr("Search profiles...")
                        background: Item {}
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeBody
                        selectByMouse: true
                        onTextChanged: Backend.profiles.setSearch(searchField.text)
                    }
                }
            }

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentHeight: Math.max(listColumn.implicitHeight, height)
                clip: true

                ColumnLayout {
                    id: listColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
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
                            onApplyClicked: {
                                if (Backend.settings.confirmBeforeApply) {
                                    applyConfirmDialog.pendingId = model.id
                                    applyConfirmDialog.pendingName = model.name
                                    applyConfirmDialog.showConfirm()
                                } else
                                    Backend.applyProfile(model.id)
                            }
                            onEditClicked: profileDialog.openForEdit(model.id)
                            onDeleteClicked: deleteDialog.showFor(model.name, model.id)
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        Layout.topMargin: Theme.spaceXl
                        spacing: Theme.spaceSm
                        visible: Backend.profiles.count === 0

                        Icon {
                            anchors.horizontalCenter: parent.horizontalCenter
                            name: "layers"
                            size: 40
                            color: Theme.textDisabled
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("No profiles yet")
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeBodyLarge
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: qsTr("Click “Add Profile” to create your first one")
                            color: Theme.textDisabled
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeCaption
                        }
                    }
                }
            }
        }
    }

    ProfileDialog {
        id: profileDialog
    }

    ConfirmDialog {
        id: deleteDialog
        property string pendingName: ""
        property string pendingId: ""
        titleText: qsTr("Delete profile")
        message: qsTr("Delete “%1”? This cannot be undone.").arg(deleteDialog.pendingName)
        confirmText: qsTr("Delete")
        confirmColor: Theme.error
        onConfirmed: {
            Backend.profiles.removeProfile(deleteDialog.pendingId)
            Backend.notify(qsTr("Profile deleted"), "success")
        }

        function showFor(name, id) {
            deleteDialog.pendingName = name
            deleteDialog.pendingId = id
            deleteDialog.open()
        }
    }

    ConfirmDialog {
        id: applyConfirmDialog
        property string pendingId: ""
        property string pendingName: ""
        titleText: qsTr("Apply profile")
        message: qsTr("Apply “%1” to the selected adapter?")
            .arg(applyConfirmDialog.pendingName)
        confirmText: qsTr("Apply")
        onConfirmed: Backend.applyProfile(applyConfirmDialog.pendingId)
    }
}
