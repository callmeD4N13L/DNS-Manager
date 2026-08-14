import QtQuick

/* --------------------------------------------------------------------------
 * DnsManager design tokens.
 *
 * Single source of truth for colors, spacing, radii and typography.
 * Reference as `Theme.<token>` after `import "theme"`.
 *
 * Toggle `dark` to switch the whole palette; colors are bindings, so every
 * UI element updates automatically.
 * ------------------------------------------------------------------------ */

QtObject {
    id: theme

    // -- Theme mode -------------------------------------------------------
    property bool dark: true
    property bool systemTheme: false

    // -- Palette ----------------------------------------------------------
    readonly property color background:   dark ? "#141519" : "#f2f3f5"
    readonly property color backgroundAlt: dark ? "#1a1b20" : "#e7e8ec"
    readonly property color surface:       dark ? "#1f2026" : "#ffffff"
    readonly property color surfaceHover:  dark ? "#262830" : "#f5f6f8"
    readonly property color surfaceBorder: dark ? "#2e3038" : "#e0e1e6"

    readonly property color textPrimary:   dark ? "#ececf0" : "#1b1c21"
    readonly property color textSecondary: dark ? "#9aa0aa" : "#5a5f69"
    readonly property color textDisabled:  dark ? "#565a63" : "#9aa0a8"

    readonly property color accent:        "#4cc2ff"
    readonly property color accentHover:   "#63cbff"
    readonly property color accentPressed: "#2fa8e8"
    readonly property color onAccent:      "#0b141c"

    readonly property color success:       "#4caf7d"
    readonly property color successDim:    dark ? "#1d3a2c" : "#d9efe3"
    readonly property color warning:       "#e5a03d"
    readonly property color warningDim:    dark ? "#3a2f1d" : "#f7ecd9"
    readonly property color error:         "#e5534b"
    readonly property color errorDim:      dark ? "#3a2122" : "#f6dfdd"

    readonly property color sidebarBg:     dark ? "#121318" : "#e9eaee"
    readonly property color overlayBg:     dark ? "#050507" : "#ffffff"

    // -- Spacing ----------------------------------------------------------
    readonly property int spaceXs: 4
    readonly property int spaceSm: 8
    readonly property int spaceMd: 12
    readonly property int spaceLg: 16
    readonly property int spaceXl: 24
    readonly property int spaceXxl: 32

    // -- Radii ------------------------------------------------------------
    readonly property int radiusSm: 6
    readonly property int radiusMd: 10
    readonly property int radiusLg: 14
    readonly property int radiusPill: 999

    // -- Typography -------------------------------------------------------
    readonly property string fontFamily: "Segoe UI Variable Text, Segoe UI"

    readonly property int fontSizeCaption: 11
    readonly property int fontSizeBody: 13
    readonly property int fontSizeBodyLarge: 15
    readonly property int fontSizeTitle: 17
    readonly property int fontSizeHeading: 22
    readonly property int fontSizeDisplay: 26

    // -- Motion -----------------------------------------------------------
    readonly property int durationFast: 120
    readonly property int durationNormal: 200

    // -- Elevation (subtle, border-based; no heavy blur effects) ----------
    readonly property color border: surfaceBorder
}
