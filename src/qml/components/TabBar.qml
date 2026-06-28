import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: tabBar
    height: 36
    color: ThemeModel.sidebar

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4
        spacing: 2

        // 示例标签页
        TabButton { text: qsTr("Database 1"); closable: true; selected: true }
        TabButton { text: qsTr("SQL Query 1"); closable: true }
    }

    component TabButton: Rectangle {
        property string text: ""
        property bool selected: false
        property bool closable: false

        Layout.preferredHeight: 28
        Layout.preferredWidth: 140
        radius: 6
        color: selected ? ThemeModel.panel : "transparent"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 6
            spacing: 4

            Text {
                text: parent.parent.text
                color: ThemeModel.text
                font.pixelSize: 12
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                visible: parent.parent.closable
                text: "×"
                color: ThemeModel.textMuted
                font.pixelSize: 14
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { /* tab switching */ }
        }
    }
}
