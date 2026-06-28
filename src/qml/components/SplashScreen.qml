import QtQuick
import QtQuick.Controls

Rectangle {
    id: splash
    anchors.fill: parent
    color: ThemeModel.bg
    z: 10000
    visible: true

    property alias progress: loadProgress.value
    property alias statusText: statusLabel.text

    function close() {
        fadeOut.start()
    }

    // 自动关闭
    Timer {
        interval: 1000
        running: true
        onTriggered: splash.close()
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "🗄️"
            font.pixelSize: 64
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "XqqytSqlite"
            color: ThemeModel.accent
            font.pixelSize: 28
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "v0.1.0 — Qt 6.8 MSVC"
            color: ThemeModel.textDim
            font.pixelSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item { height: 20 }

        ProgressBar {
            id: loadProgress
            width: 260; height: 6
            value: 0.0
            anchors.horizontalCenter: parent.horizontalCenter
            background: Rectangle { radius: 3; color: ThemeModel.border }
            contentItem: Rectangle {
                radius: 3; color: ThemeModel.accent
                width: loadProgress.visualPosition * parent.width
            }
        }

        Text {
            id: statusLabel
            text: qsTr("Starting...")
            color: ThemeModel.textMuted
            font.pixelSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    NumberAnimation {
        id: fadeOut
        target: splash
        property: "opacity"
        from: 1.0; to: 0.0
        duration: 500
        onFinished: splash.visible = false
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }
}
