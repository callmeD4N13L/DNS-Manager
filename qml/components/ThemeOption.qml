import QtQuick

import "../theme"
import "."

/* Segmented-control style option button used by the Settings theme picker. */
PrimaryButton {
    property bool optionSelected: false
    accentColor: optionSelected ? Theme.accent : Theme.surfaceHover
    accentText: optionSelected ? Theme.onAccent : Theme.textSecondary
    implicitHeight: 30
    leftPadding: 14
    rightPadding: 14
}
