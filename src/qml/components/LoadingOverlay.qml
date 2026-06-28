import QtQuick
import QtQuick.Controls

// 加载遮罩——半透明 + 旋转指示器
Rectangle {
    id: overlay

    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.4)
    visible: false
    z: 999

    property alias message: label.text

    function show(msg) {
        label.text = msg || ""
        overlay.visible = true
    }

    function hide() {
        overlay.visible = false
    }

    MouseArea {
        anchors.fill: parent  // 阻止穿透点击
    }

    Rectangle {
        width: 160
        height: 120
        radius: ThemeModel.cardRadius
        color: ThemeModel.panel
        anchors.centerIn: parent

        Column {
            anchors.centerIn: parent
            spacing: 12

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                running: overlay.visible
            }

            Text {
                id: label
                anchors.horizontalCenter: parent.horizontalCenter
                text: ""
                color: ThemeModel.textDim
                font.pixelSize: 12
            }
        }
    }
}
