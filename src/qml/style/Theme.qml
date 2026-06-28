pragma Singleton

import QtQuick

QtObject {
    // 获取 C++ ThemeModel 单例
    property var theme: ThemeModel

    // 颜色
    property color bg: theme.bg
    property color sidebar: theme.sidebar
    property color panel: theme.panel
    property color border: theme.border
    property color accent: theme.accent
    property color accent2: theme.accent2
    property color text: theme.text
    property color textMuted: theme.textMuted
    property color textDim: theme.textDim
    property color success: theme.success
    property color error: theme.error
    property color warning: theme.warning
    property color info: theme.info
    property color hover: theme.hover
    property color selected: theme.selected

    // 阴影
    property string raisedShadow: theme.raisedShadow
    property string insetShadow: theme.insetShadow

    // 圆角
    property int cardRadius: 16
    property int buttonRadius: 12
    property int badgeRadius: 8

    // 字号
    property int fontSize: 13
    property int fontSizeSmall: 11
    property int fontSizeLarge: 16

    // 间距
    property int spacingSmall: 4
    property int spacingMedium: 8
    property int spacingLarge: 16
}
