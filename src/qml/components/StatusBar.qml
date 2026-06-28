import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: statusBar
    height: 28
    color: ThemeModel.sidebar

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: ThemeModel.border
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 16

        Text {
            text: qsTr("Disconnected")
            color: ThemeModel.textMuted
            font.pixelSize: 11
        }

        Text {
            text: qsTr("Mode: Remote")
            color: ThemeModel.textMuted
            font.pixelSize: 11
        }

        Item { Layout.fillWidth: true }

        Text {
            text: qsTr("Rows: 0")
            color: ThemeModel.textMuted
            font.pixelSize: 11
        }

        Text {
            text: qsTr("Time: 0ms")
            color: ThemeModel.textMuted
            font.pixelSize: 11
        }
    }
}
