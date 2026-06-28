import QtQuick
import QtQuick.Controls

// 轻提示——底部弹出，自动消失
Rectangle {
    id: toast

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 80
    width: toastText.implicitWidth + 32
    height: 40
    radius: ThemeModel.buttonRadius
    color: ThemeModel.panel
    border.color: ThemeModel.border
    visible: false
    opacity: 0
    z: 1000

    property alias text: toastText.text
    property int duration: 3000

    function show(msg, type) {
        toastText.text = msg
        if (type === "error") {
            toast.color = ThemeModel.error
        } else if (type === "success") {
            toast.color = ThemeModel.success
        } else {
            toast.color = ThemeModel.panel
        }
        toast.visible = true
        toast.opacity = 1
        hideTimer.restart()
    }

    Text {
        id: toastText
        anchors.centerIn: parent
        color: ThemeModel.text
        font.pixelSize: 13
    }

    Timer {
        id: hideTimer
        interval: toast.duration
        onTriggered: {
            toast.opacity = 0
            hideDelay.start()
        }
    }

    Timer {
        id: hideDelay
        interval: 300
        onTriggered: toast.visible = false
    }

    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }
}
