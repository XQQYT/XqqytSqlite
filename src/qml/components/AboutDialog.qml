import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: aboutDialog
    title: qsTr("About XqqytSqlite-Qt")
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        radius: ThemeModel.cardRadius
        color: ThemeModel.panel
        border.color: ThemeModel.border
    }

    contentItem: ColumnLayout {
        width: 380; spacing: 16

        Text {
            text: "🗄️"
            font.pixelSize: 48
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "XqqytSqlite-Qt"
            color: ThemeModel.text
            font.pixelSize: 20; font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("Version 0.1.0")
            color: ThemeModel.textDim; font.pixelSize: 13
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("A remote SQLite database management tool built with Qt 6.8 (QML + C++17).\n\nFeatures:\n• SSH remote command mode\n• SFTP sync mode with conflict detection\n• SQL editor with syntax highlighting & auto-complete\n• Visual table designer with DDL preview\n• Neumorphism dark/light theme\n\nTech Stack: Qt 6.8 · LibSSH2 · sqlite3 · nlohmann/json · spdlog\nLicense: MIT")
            color: ThemeModel.textDim; font.pixelSize: 13
            lineHeight: 1.5; wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.preferredHeight: 1; Layout.fillWidth: true
            color: ThemeModel.border
        }

        Text {
            text: qsTr("Keyboard Shortcuts") + "\n" +
                  "Ctrl+Enter  " + qsTr("Execute SQL") + "\n" +
                  "Ctrl+Space  " + qsTr("Auto-complete") + "\n" +
                  "Ctrl+/      " + qsTr("Toggle comment") + "\n" +
                  "Ctrl+N      " + qsTr("New connection") + "\n" +
                  "Ctrl+,      " + qsTr("Settings") + "\n" +
                  "Ctrl+Q      " + qsTr("Quit")
            color: ThemeModel.textDim; font.pixelSize: 12
            lineHeight: 1.6
            Layout.fillWidth: true
        }

        NeumorphismButton {
            text: qsTr("Close"); implicitWidth: 100
            Layout.alignment: Qt.AlignHCenter
            onClicked: aboutDialog.close()
        }
    }
}
