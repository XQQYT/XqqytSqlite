import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    // StackView 子页面必须用 implicit size
    implicitWidth: 600
    implicitHeight: 400

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Text {
            text: qsTr("Settings")
            color: ThemeModel.text
            font.pixelSize: 22
            font.bold: true
        }

        // === 外观 ===
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: col1.implicitHeight + 40
            radius: 16
            color: ThemeModel.panel
            border.color: ThemeModel.border

            ColumnLayout {
                id: col1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 20
                spacing: 12

                Text {
                    text: qsTr("Appearance")
                    color: ThemeModel.text
                    font.pixelSize: 16
                    font.bold: true
                }

                // 主题行
                RowLayout {
                    spacing: 12
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Theme:")
                        color: ThemeModel.textDim
                        font.pixelSize: 14
                        Layout.preferredWidth: 90
                    }
                    ComboBox {
                        id: themeCombo
                        model: [qsTr("Dark"), qsTr("Light")]
                        currentIndex: ThemeModel.isDark() ? 0 : 1
                        Layout.preferredWidth: 120
                        background: Rectangle {
                            radius: 10
                            color: ThemeModel.panel
                            border.color: ThemeModel.border
                        }
                        contentItem: Text {
                            text: themeCombo.currentText
                            color: ThemeModel.text
                            font.pixelSize: 13
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }
                        onCurrentIndexChanged: {
                            if (typeof AppBridge !== 'undefined')
                                AppBridge.setTheme(currentIndex === 0 ? "dark" : "light")
                        }
                    }
                }

                // 语言行
                RowLayout {
                    spacing: 12
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Language:")
                        color: ThemeModel.textDim
                        font.pixelSize: 14
                        Layout.preferredWidth: 90
                    }
                    ComboBox {
                        id: langCombo
                        model: ["中文", "English"]
                        currentIndex: 0
                        Layout.preferredWidth: 120
                        background: Rectangle {
                            radius: 10
                            color: ThemeModel.panel
                            border.color: ThemeModel.border
                        }
                        contentItem: Text {
                            text: langCombo.currentText
                            color: ThemeModel.text
                            font.pixelSize: 13
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }
                        onCurrentIndexChanged: {
                            if (typeof I18n !== 'undefined')
                                I18n.switchLanguage(currentIndex === 0 ? "zh" : "en")
                        }
                    }
                }
            }
        }

        // === 查询 ===
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: col2.implicitHeight + 40
            radius: 16
            color: ThemeModel.panel
            border.color: ThemeModel.border

            ColumnLayout {
                id: col2
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 20
                spacing: 12

                Text {
                    text: qsTr("Query Settings")
                    color: ThemeModel.text
                    font.pixelSize: 16
                    font.bold: true
                }

                RowLayout {
                    spacing: 12
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Page size:")
                        color: ThemeModel.textDim
                        font.pixelSize: 14
                        Layout.preferredWidth: 90
                    }
                    SpinBox {
                        id: pageSizeSpin
                        from: 10
                        to: 1000
                        stepSize: 10
                        value: 100
                        editable: true
                        Layout.preferredWidth: 120
                        background: Rectangle {
                            radius: 10
                            color: ThemeModel.panel
                            border.color: ThemeModel.border
                        }
                        contentItem: TextInput {
                            text: pageSizeSpin.value
                            color: ThemeModel.text
                            font.pixelSize: 13
                            horizontalAlignment: TextInput.AlignHCenter
                            verticalAlignment: TextInput.AlignVCenter
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
