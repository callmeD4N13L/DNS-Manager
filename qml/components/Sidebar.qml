import QtQuick
import QtQuick.Layouts

import "../theme"
import "."

/* Left navigation rail: brand, page entries, footer. */
Rectangle {
    id: root

    property int currentPage: 0
    property var pages: [
        { text: qsTr("Dashboard"), icon: "dashboard" },
        { text: qsTr("DNS Profiles"), icon: "layers" },
        { text: qsTr("Network"), icon: "wifi" },
        { text: qsTr("Settings"), icon: "settings" }
    ]

    signal pageRequested(int index)

    width: 224
    color: Theme.sidebarBg

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spaceXl
            Layout.bottomMargin: Theme.spaceXl
            Layout.leftMargin: Theme.spaceLg
            Layout.rightMargin: Theme.spaceLg
            spacing: Theme.spaceMd

            Rectangle {
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: Theme.surfaceBorder

                Image {
                    anchors.fill: parent
                    anchors.margins: 5
                    source: "qrc:/icons/app/app-icon-256.png"
                    sourceSize: Qt.size(24, 24)
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Text {
                    text: qsTr("DNS Manager")
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeTitle
                    font.weight: Font.DemiBold
                }

                Text {
                    text: qsTr("v%1").arg(Qt.applicationVersion)
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeCaption
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spaceMd
            Layout.rightMargin: Theme.spaceMd
            spacing: 2

            Repeater {
                model: root.pages

                SidebarItem {
                    Layout.fillWidth: true
                    text: modelData.text
                    iconName: modelData.icon
                    selected: index === root.currentPage
                    onClicked: root.pageRequested(index)
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Text {
            Layout.leftMargin: Theme.spaceLg
            Layout.bottomMargin: Theme.spaceLg
            text: qsTr("Windows 10 / 11")
            color: Theme.textDisabled
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeCaption
        }
    }
}
