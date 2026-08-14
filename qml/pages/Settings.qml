import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import "../components"
import "../theme"

ColumnLayout {
    id: root

    spacing: Theme.spaceXl

    PageHeader {
        title: qsTr("Settings")
        subtitle: qsTr("Application preferences")
    }

    Card {
        Layout.fillWidth: true
        title: qsTr("Appearance")
        subtitle: qsTr("Theme selection")

        ColumnLayout {
            spacing: Theme.spaceXl

            SettingRow {
                label: qsTr("Theme")
                description: qsTr("System follows the Windows light/dark setting")
                control: Component {
                    RowLayout {
                        spacing: Theme.spaceSm

                        ThemeOption {
                            text: qsTr("System")
                            optionSelected: Backend.settings.themeMode === "system"
                            onClicked: Backend.settings.themeMode = "system"
                        }
                        ThemeOption {
                            text: qsTr("Dark")
                            optionSelected: Backend.settings.themeMode === "dark"
                            onClicked: Backend.settings.themeMode = "dark"
                        }
                        ThemeOption {
                            text: qsTr("Light")
                            optionSelected: Backend.settings.themeMode === "light"
                            onClicked: Backend.settings.themeMode = "light"
                        }
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: qsTr("Startup & tray")
        subtitle: qsTr("Launch and background behavior")

        ColumnLayout {
            spacing: Theme.spaceXl

            SettingRow {
                label: qsTr("Start with Windows")
                description: qsTr("Launch DNS Manager automatically when you sign in")
                control: Component {
                    ToggleSwitch {
                        text: ""
                        checked: Backend.settings.startWithWindows
                        onToggled: Backend.settings.startWithWindows = checked
                    }
                }
            }

            SettingRow {
                label: qsTr("Launch minimized")
                description: qsTr("Start hidden in the system tray")
                control: Component {
                    ToggleSwitch {
                        text: ""
                        checked: Backend.settings.startMinimized
                        onToggled: Backend.settings.startMinimized = checked
                    }
                }
            }

            SettingRow {
                label: qsTr("Minimize to system tray")
                description: qsTr("Keep running in the tray when the window is closed")
                control: Component {
                    ToggleSwitch {
                        text: ""
                        checked: Backend.settings.minimizeToTray
                        onToggled: Backend.settings.minimizeToTray = checked
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: qsTr("DNS behavior")
        subtitle: qsTr("What happens after you apply a configuration")

        ColumnLayout {
            spacing: Theme.spaceXl

            SettingRow {
                label: qsTr("Flush DNS cache after applying")
                description: qsTr("Run the resolver-cache flush after every successful change")
                control: Component {
                    ToggleSwitch {
                        text: ""
                        checked: Backend.settings.flushCacheAfterApply
                        onToggled: Backend.settings.flushCacheAfterApply = checked
                    }
                }
            }

            SettingRow {
                label: qsTr("Confirm before changing DNS")
                description: qsTr("Ask for confirmation before applying or resetting")
                control: Component {
                    ToggleSwitch {
                        text: ""
                        checked: Backend.settings.confirmBeforeApply
                        onToggled: Backend.settings.confirmBeforeApply = checked
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: qsTr("Data")
        subtitle: qsTr("Profiles and configuration")

        ColumnLayout {
            spacing: Theme.spaceXl

            SettingRow {
                label: qsTr("Import profiles")
                description: qsTr("Merge DNS profiles from a JSON file")
                control: Component {
                    SecondaryButton {
                        text: qsTr("Import...")
                        onClicked: importDialog.open()
                    }
                }
            }

            SettingRow {
                label: qsTr("Export profiles")
                description: qsTr("Save all profiles to a JSON file")
                control: Component {
                    SecondaryButton {
                        text: qsTr("Export...")
                        onClicked: exportDialog.open()
                    }
                }
            }

            SettingRow {
                label: qsTr("Reset configuration")
                description: qsTr("Restore default profiles and settings")
                control: Component {
                    SecondaryButton {
                        text: qsTr("Reset")
                        onClicked: resetDialog.showConfirm()
                    }
                }
            }

            SettingRow {
                label: qsTr("Profiles file")
                description: qsTr("Where DNS profiles are stored")
                control: Component {
                    Text {
                        text: Backend.profilesFilePath
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeCaption
                        elide: Text.ElideMiddle
                        Layout.preferredWidth: 260
                    }
                }
            }

            SettingRow {
                label: qsTr("Settings file")
                description: qsTr("Where application preferences are stored")
                control: Component {
                    Text {
                        text: Backend.settingsFilePath
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeCaption
                        elide: Text.ElideMiddle
                        Layout.preferredWidth: 260
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: qsTr("About")
        subtitle: qsTr("Version information")

        ColumnLayout {
            spacing: Theme.spaceSm

            SettingRow {
                label: qsTr("Application")
                control: Component {
                    Text {
                        text: qsTr("DNS Manager %1").arg(Backend.version)
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeBody
                        font.weight: Font.Medium
                    }
                }
            }

            SettingRow {
                label: qsTr("Platform")
                control: Component {
                    Text {
                        text: "%1 / Qt %2".arg(Backend.platform, Backend.qtVersion)
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeBody
                    }
                }
            }

            Text {
                text: Backend.aboutText
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeCaption
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.topMargin: Theme.spaceSm
            }
        }
    }

    Item { Layout.fillHeight: true }

    FileDialog {
        id: importDialog
        title: qsTr("Import profiles")
        nameFilters: ["DNS profiles (*.json)"]
        onAccepted: Backend.importProfiles(importDialog.selectedFile)
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export profiles")
        fileMode: FileDialog.SaveFile
        nameFilters: ["DNS profiles (*.json)"]
        defaultSuffix: "json"
        onAccepted: Backend.exportProfiles(exportDialog.selectedFile)
    }

    ConfirmDialog {
        id: resetDialog
        titleText: qsTr("Reset configuration")
        message: qsTr("Restore default profiles and settings? Current profiles will be replaced.")
        confirmText: qsTr("Reset")
        confirmColor: Theme.error
        onConfirmed: Backend.resetConfiguration()
    }
}
