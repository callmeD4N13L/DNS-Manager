import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"
import "."

/* Modal dialog to create or edit a DNS profile.
 * Opens with openForAdd() or openForEdit(id); saves through Backend.profiles
 * and reports the outcome through the toast system. */
Dialog {
    id: root

    property string editingId: ""
    property bool isFavorite: false

    readonly property bool isEditing: root.editingId.length > 0

    title: root.isEditing ? qsTr("Edit Profile") : qsTr("New Profile")
    modal: true
    width: 480
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
            text: root.title
            color: Theme.textPrimary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeTitle
            font.weight: Font.DemiBold
        }
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
                text: root.isEditing ? qsTr("Save changes") : qsTr("Create")
                onClicked: root.save()
            }
        }
    }

    contentItem: Item {
        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spaceXl
            anchors.rightMargin: Theme.spaceXl
            anchors.topMargin: Theme.spaceLg
            anchors.bottomMargin: Theme.spaceLg
            spacing: Theme.spaceMd

        FormField {
            id: nameField
            Layout.fillWidth: true
            label: qsTr("Name")
            placeholder: qsTr("e.g. My Work DNS")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spaceMd

            FormField {
                id: ipv4PrimaryField
                Layout.fillWidth: true
                label: qsTr("Primary IPv4")
                placeholder: qsTr("1.1.1.1")
                monospace: true
            }

            FormField {
                id: ipv4SecondaryField
                Layout.fillWidth: true
                label: qsTr("Secondary IPv4")
                placeholder: qsTr("1.0.0.1")
                monospace: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spaceMd

            FormField {
                id: ipv6PrimaryField
                Layout.fillWidth: true
                label: qsTr("Primary IPv6")
                placeholder: qsTr("2606:4700:4700::1111")
                monospace: true
            }

            FormField {
                id: ipv6SecondaryField
                Layout.fillWidth: true
                label: qsTr("Secondary IPv6")
                placeholder: qsTr("2606:4700:4700::1001")
                monospace: true
            }
        }

        FormField {
            id: providerField
            Layout.fillWidth: true
            label: qsTr("Provider")
            placeholder: qsTr("e.g. Cloudflare, Inc.")
        }

        FormField {
            id: descriptionField
            Layout.fillWidth: true
            label: qsTr("Description")
            placeholder: qsTr("Optional notes")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spaceXs

            Text {
                text: qsTr("Mark as favorite")
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeBody
            }

            Item { Layout.fillWidth: true }

            IconButton {
                iconName: "star"
                iconColor: root.isFavorite ? Theme.warning : Theme.textDisabled
                onClicked: root.isFavorite = !root.isFavorite
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Toggle favorite")
            }
        }
    }
    }

    function openForAdd() {
        root.editingId = ""
        root.isFavorite = false
        nameField.fieldValue = ""
        providerField.fieldValue = ""
        descriptionField.fieldValue = ""
        ipv4PrimaryField.fieldValue = ""
        ipv4SecondaryField.fieldValue = ""
        ipv6PrimaryField.fieldValue = ""
        ipv6SecondaryField.fieldValue = ""
        root.open()
    }

    function openForEdit(id) {
        const list = Backend.profiles.profilesList()
        for (let i = 0; i < list.length; ++i) {
            if (list[i].id !== id)
                continue
            root.editingId = id
            root.isFavorite = list[i].favorite
            nameField.fieldValue = list[i].name
            providerField.fieldValue = list[i].provider
            descriptionField.fieldValue = list[i].description
            ipv4PrimaryField.fieldValue = list[i].primaryIpv4
            ipv4SecondaryField.fieldValue = list[i].secondaryIpv4
            ipv6PrimaryField.fieldValue = list[i].primaryIpv6
            ipv6SecondaryField.fieldValue = list[i].secondaryIpv6
            root.open()
            return
        }
    }

    function save() {
        const name = nameField.fieldValue.trim()
        if (name.length === 0) {
            Backend.notify(qsTr("Profile name is required"), "error")
            return
        }

        const fields = {
            "name": name,
            "provider": providerField.fieldValue,
            "description": descriptionField.fieldValue,
            "primaryIpv4": ipv4PrimaryField.fieldValue.trim(),
            "secondaryIpv4": ipv4SecondaryField.fieldValue.trim(),
            "primaryIpv6": ipv6PrimaryField.fieldValue.trim(),
            "secondaryIpv6": ipv6SecondaryField.fieldValue.trim(),
            "favorite": root.isFavorite
        }

        const ok = root.isEditing
            ? Backend.profiles.updateProfile(root.editingId, fields)
            : Backend.profiles.addProfile(fields).length > 0

        if (ok) {
            Backend.notify(root.isEditing ? qsTr("Profile updated") : qsTr("Profile created"), "success")
            root.close()
        } else {
            Backend.notify(qsTr("Enter a valid name and at least one valid DNS server"), "error")
        }
    }
}