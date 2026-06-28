import QtQuick
import QtQuick.Controls

MenuBar {
    id: menuBar

    background: Rectangle {
        color: ThemeModel.sidebar
        border.color: ThemeModel.border
        border.width: 0
        height: 1
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: ThemeModel.border
        }
    }

    Menu {
        title: qsTr("File")
        Action { text: qsTr("New Connection"); shortcut: "Ctrl+N" }
        Action { text: qsTr("Open Database..."); shortcut: "Ctrl+O" }
        MenuSeparator {}
        Action { text: qsTr("Settings"); shortcut: "Ctrl+," }
        MenuSeparator {}
        Action { text: qsTr("Exit"); shortcut: "Ctrl+Q" }
    }

    Menu {
        title: qsTr("Connection")
        Action { text: qsTr("Connect...") }
        Action { text: qsTr("Disconnect") }
        MenuSeparator {}
        Action { text: qsTr("Test Connection") }
        Action { text: qsTr("Manage Connections...") }
    }

    Menu {
        title: qsTr("View")
        Action { text: qsTr("Toggle Theme") }
        Action { text: qsTr("Toggle Sidebar") }
        MenuSeparator {}
        Action { text: qsTr("Zoom In"); shortcut: "Ctrl+=" }
        Action { text: qsTr("Zoom Out"); shortcut: "Ctrl+-" }
    }

    Menu {
        title: qsTr("Tools")
        Action { text: qsTr("SQL Editor"); shortcut: "Ctrl+E" }
        Action { text: qsTr("Table Designer") }
        Action { text: qsTr("SFTP Manager") }
    }

    Menu {
        title: qsTr("Help")
        Action { text: qsTr("About") }
        Action { text: qsTr("Documentation") }
    }
}
