import QtQuick
import QtQuick.Controls

// 拟态化按钮——默认凸起，按下时凹陷
Button {
    id: btn

    property bool accent: false
    property int btnRadius: ThemeModel.buttonRadius

    implicitWidth: 120
    implicitHeight: 36

    // 按钮背景
    background: Rectangle {
        id: bg
        radius: btn.btnRadius
        color: {
            if (btn.accent) {
                return btn.pressed
                    ? Qt.lighter(ThemeModel.accent, 1.2)
                    : ThemeModel.accent
            }
            return btn.pressed
                ? Qt.darker(ThemeModel.panel, 1.05)
                : ThemeModel.panel
        }
        border.width: 1
        border.color: ThemeModel.border

        // 凸起阴影（未按下）或 凹陷阴影（按下）
        state: btn.pressed ? "pressed" : "normal"

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            visible: bg.state == "normal"

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: Qt.rgba(0, 0, 0, 0.25)
                anchors.topMargin: 2
                anchors.leftMargin: 2
                anchors.rightMargin: -2
                anchors.bottomMargin: -2
                z: -1
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            visible: bg.state == "pressed"

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: Qt.rgba(0, 0, 0, 0.30)
                anchors.topMargin: -1
                anchors.leftMargin: -1
                anchors.rightMargin: 1
                anchors.bottomMargin: 1
                z: -1
            }
        }

        Behavior on color {
            ColorAnimation { duration: 100 }
        }
    }

    contentItem: Text {
        text: btn.text
        font.pixelSize: ThemeModel.fontSize
        color: btn.accent && !btn.pressed ? "#ffffff"
                                          : ThemeModel.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
}
