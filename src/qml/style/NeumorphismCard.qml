import QtQuick

// 拟态化凸起卡片——convex 阴影（默认）或凹陷（inset）
Rectangle {
    id: card

    property bool inset: false
    property bool hoverable: false
    property alias hovered: mouseArea.containsMouse

    implicitWidth: 200
    implicitHeight: 120
    radius: ThemeModel.cardRadius
    color: ThemeModel.panel
    border.width: 1
    border.color: ThemeModel.border

    // 凸起阴影层
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        visible: !card.inset

        // 右下暗影
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.rgba(0, 0, 0, 0.25)
            anchors.topMargin: 4
            anchors.leftMargin: 4
            anchors.rightMargin: -4
            anchors.bottomMargin: -4
            z: -1
        }

        // 左上亮边
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.rgba(1, 1, 1, 0.03)
            anchors.topMargin: -4
            anchors.leftMargin: -4
            anchors.rightMargin: 4
            anchors.bottomMargin: 4
            z: -1
        }
    }

    // 凹陷阴影层
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        visible: card.inset

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.rgba(0, 0, 0, 0.25)
            anchors.topMargin: -3
            anchors.leftMargin: -3
            anchors.rightMargin: 3
            anchors.bottomMargin: 3
            z: -1
        }
    }

    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: card.hoverable
        onHoveredChanged: {
            if (card.hoverable) {
                card.color = hovered ? ThemeModel.instance().hover
                                     : ThemeModel.instance().panel
            }
        }
    }
}
