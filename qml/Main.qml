import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import "components"
import "pages"
import "theme"

Window {
    id: root

    width: 1280
    height: 800
    minimumWidth: 960
    minimumHeight: 620

    visible: !Backend.startHidden
    color: Theme.background
    title: qsTr("DNS Manager")
    icon: "qrc:/icons/app/app-icon-256.png"
    font.family: Theme.fontFamily

    property int currentPage: 0

    // Keep the themed palette in sync with the effective dark-mode setting
    // (which itself follows settings + the Windows color scheme).
    Binding {
        target: Theme
        property: "dark"
        value: Backend.darkMode
    }

    Component.onCompleted: Backend.setMainWindow(root)

    // Close = hide to tray unless the user explicitly quit from the tray.
    onClosing: (close) => {
        if (Backend.settings.minimizeToTray && !Backend.isQuitting) {
            close.accepted = false
            Backend.hideToTray()
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            id: sidebar
            currentPage: root.currentPage
            onPageRequested: (index) => root.currentPage = index
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StackLayout {
                anchors.fill: parent
                anchors.margins: Theme.spaceXl
                currentIndex: root.currentPage

                Dashboard {
                    activePage: root.currentPage
                }

                Profiles {}

                Network {}

                Settings {}
            }
        }
    }

    // --- Toast -----------------------------------------------------------
    Item {
        id: toastLayer
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.spaceXl
        width: Math.min(460, parent.width - Theme.spaceXxl)

        opacity: 0
        visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: Theme.durationFast } }

        function show(text, kind) {
            const isError = kind === "error";
            const isSuccess = kind === "success";
            toastIcon.name = isError ? "warning" : isSuccess ? "check" : "info";
            toastIcon.color = isError ? Theme.error : isSuccess ? Theme.success : Theme.accent;
            toastRect.border.color = toastIcon.color;
            toastRect.color = isError ? Theme.errorDim : isSuccess ? Theme.successDim : Theme.surfaceHover;
            toastLabel.text = text;
            toastTimer.restart();
            toastLayer.opacity = 1;
        }

        Rectangle {
            id: toastRect
            width: parent.width
            height: toastContent.implicitHeight + Theme.spaceLg * 2
            radius: Theme.radiusMd
            border.width: 1

            RowLayout {
                id: toastContent
                anchors.fill: parent
                anchors.margins: Theme.spaceLg
                spacing: Theme.spaceMd

                Icon {
                    id: toastIcon
                    size: 16
                }

                Text {
                    id: toastLabel
                    Layout.fillWidth: true
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeBody
                    wrapMode: Text.WordWrap
                }
            }
        }

        Timer {
            id: toastTimer
            interval: 3200
            onTriggered: toastLayer.opacity = 0
        }
    }

    Connections {
        target: Backend
        function onNotificationRequested(text, kind) {
            toastLayer.show(text, kind);
        }
    }
}
