import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sideNav

    color: ThemeModel.sidebar
    border.color: ThemeModel.border
    border.width: 0

    // 页面切换信号：0=SFTP, 1=数据浏览, 2=SQL编辑器, 3=建表向导
    signal pageSelected(int index)

    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: ThemeModel.border
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 4

        NavButton {
            icon.text: "📡"
            tooltip: qsTr("Connections")
            selected: root.currentPageIndex === -1
            onClicked: sideNav.pageSelected(-1)
        }

        NavButton {
            icon.text: "📁"
            tooltip: qsTr("SFTP Manager")
            selected: root.currentPageIndex === 0
            onClicked: sideNav.pageSelected(0)
        }

        NavButton {
            icon.text: "🗄️"
            tooltip: qsTr("Database Browser")
            selected: root.currentPageIndex === 1
            onClicked: sideNav.pageSelected(1)
        }

        NavButton {
            icon.text: "📝"
            tooltip: qsTr("SQL Editor")
            selected: root.currentPageIndex === 2
            onClicked: sideNav.pageSelected(2)
        }

        NavButton {
            icon.text: "🔧"
            tooltip: qsTr("Table Designer")
            selected: root.currentPageIndex === 3
            onClicked: sideNav.pageSelected(3)
        }

        Item { Layout.fillHeight: true }

        // 底部按钮
        NavButton {
            icon.text: "⚙️"
            tooltip: qsTr("Settings")
            onClicked: sideNav.pageSelected(4)
        }

        NavButton {
            icon.text: ThemeModel.isDark() ? "☀️" : "🌙"
            tooltip: qsTr("Toggle Theme")
            onClicked: {
                if (typeof AppBridge !== 'undefined') {
                    AppBridge.toggleTheme()
                } else {
                    ThemeModel.toggleTheme()
                }
            }
        }
    }

    component NavButton: Rectangle {
        Layout.preferredWidth: 48
        Layout.preferredHeight: 48
        Layout.alignment: Qt.AlignHCenter
        radius: ThemeModel.buttonRadius
        color: selected ? ThemeModel.selected
               : (mouseArea.containsMouse ? ThemeModel.hover : "transparent")

        property alias icon: iconText
        property alias tooltip: tooltipText.text
        property bool selected: false
        property bool hovered: false
        signal clicked()

        Text {
            id: iconText
            anchors.centerIn: parent
            font.pixelSize: 20
        }

        ToolTip {
            id: tooltipText
            visible: mouseArea.containsMouse
            delay: 500
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }
}
