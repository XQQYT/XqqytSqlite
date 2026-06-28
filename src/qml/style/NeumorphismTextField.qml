import QtQuick
import QtQuick.Controls

// 拟态化输入框——凹陷样式
TextField {
    id: textField

    property int fieldRadius: ThemeModel.buttonRadius

    implicitWidth: 200
    implicitHeight: 36

    font.pixelSize: ThemeModel.fontSize
    color: ThemeModel.text
    placeholderTextColor: ThemeModel.textMuted

    leftPadding: 12
    rightPadding: 12

    background: Rectangle {
        radius: textField.fieldRadius
        color: Qt.darker(ThemeModel.panel, 1.05)
        border.width: 1
        border.color: textField.activeFocus
            ? ThemeModel.accent
            : ThemeModel.border

        // 凹陷阴影
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: Qt.rgba(0, 0, 0, 0.25)
                anchors.topMargin: -2
                anchors.leftMargin: -2
                anchors.rightMargin: 2
                anchors.bottomMargin: 2
                z: -1
            }
        }

        Behavior on border.color {
            ColorAnimation { duration: 200 }
        }
    }
}
