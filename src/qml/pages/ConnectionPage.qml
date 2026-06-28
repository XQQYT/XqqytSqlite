import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Text {
            text: qsTr("SSH Connection Manager")
            color: ThemeModel.text
            font.pixelSize: 20
            font.bold: true
        }

        // 新建连接表单
        NeumorphismCard {
            id: connectionCard
            Layout.fillWidth: true
            Layout.preferredHeight: 380
            implicitHeight: 380

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Text { text: qsTr("New Connection"); color: ThemeModel.text; font.pixelSize: 16 }
                Text { text: qsTr("Connect to a remote device via SSH"); color: ThemeModel.textDim; font.pixelSize: 12 }

                GridLayout {
                    columns: 3
                    rowSpacing: 8
                    columnSpacing: 12
                    Layout.fillWidth: true

                    // Host
                    Text { text: qsTr("Host:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                    NeumorphismTextField {
                        id: hostField; placeholderText: qsTr("hostname or IP")
                        Layout.fillWidth: true; Layout.columnSpan: 2
                    }

                    // Port
                    Text { text: qsTr("Port:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                    NeumorphismTextField {
                        id: portField; text: "22"; implicitWidth: 80
                    }
                    Item {}

                    // Username
                    Text { text: qsTr("User:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                    NeumorphismTextField {
                        id: userField; placeholderText: qsTr("username")
                        Layout.fillWidth: true; Layout.columnSpan: 2
                    }

                    // Auth method
                    Text { text: qsTr("Auth:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                    ComboBox {
                        id: authMethod
                        model: [qsTr("Password"), qsTr("Private Key"), qsTr("SSH Agent")]
                        Layout.preferredWidth: 140
                        background: Rectangle {
                            radius: ThemeModel.buttonRadius
                            color: ThemeModel.panel
                            border.color: ThemeModel.border
                        }
                        contentItem: Text {
                            text: authMethod.currentText
                            color: ThemeModel.text
                            font.pixelSize: 13
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }
                        onCurrentIndexChanged: {
                            passwordRow.visible = (currentIndex === 0)
                            keyRow.visible = (currentIndex === 1)
                        }
                    }
                    Item {}

                    // Password row
                    RowLayout {
                        id: passwordRow
                        Layout.columnSpan: 3; Layout.fillWidth: true
                        spacing: 12; visible: true
                        Text { text: qsTr("Pass:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                        NeumorphismTextField {
                            id: passField; echoMode: TextInput.Password
                            placeholderText: qsTr("password"); Layout.fillWidth: true
                        }
                    }

                    // Key file row
                    RowLayout {
                        id: keyRow
                        Layout.columnSpan: 3; Layout.fillWidth: true
                        spacing: 12; visible: false
                        Text { text: qsTr("Key:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                        NeumorphismTextField {
                            id: keyField; placeholderText: qsTr("path to private key"); Layout.fillWidth: true
                        }
                        NeumorphismButton { text: "..."; implicitWidth: 40; implicitHeight: 36 }
                    }

                    // Passphrase row
                    RowLayout {
                        Layout.columnSpan: 3; Layout.fillWidth: true; spacing: 12
                        Text { text: qsTr("Passphrase:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                        NeumorphismTextField {
                            id: passphraseField; echoMode: TextInput.Password
                            placeholderText: qsTr("optional"); Layout.fillWidth: true
                        }
                    }

                    // Jump host
                    RowLayout {
                        Layout.columnSpan: 3; Layout.fillWidth: true; spacing: 12
                        Text { text: qsTr("Jump:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                        NeumorphismTextField {
                            id: jumpField
                            placeholderText: qsTr("user@host:port (optional)")
                            Layout.fillWidth: true
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // 操作按钮
                RowLayout {
                    spacing: 12
                    Layout.alignment: Qt.AlignRight

                    NeumorphismButton {
                        text: qsTr("Test Connection")
                        implicitWidth: 140
                        onClicked: {
                            if (typeof AppBridge !== 'undefined') {
                                _loading.show(qsTr("Testing connection..."))
                                AppBridge.testConnection(
                                    hostField.text, userField.text,
                                    passField.text, parseInt(portField.text) || 22
                                )
                                _loading.hide()
                            }
                        }
                    }

                    NeumorphismButton {
                        text: qsTr("Connect")
                        accent: true
                        implicitWidth: 120
                        onClicked: {
                            if (typeof AppBridge !== 'undefined') {
                                _loading.show(qsTr("Connecting..."))
                                var id = AppBridge.connectToHost(
                                    hostField.text, userField.text,
                                    passField.text, parseInt(portField.text) || 22
                                )
                                _loading.hide()
                                if (id) {
                                    _toast.show(qsTr("Connected: ") + hostField.text, "success")
                                }
                            }
                        }
                    }
                }
            }
        }

        // 已保存连接
        Text {
            text: qsTr("Saved Connections")
            color: ThemeModel.text
            font.pixelSize: 16
            font.bold: true
        }

        NeumorphismCard {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                anchors.fill: parent
                anchors.margins: 8
                model: typeof AppBridge !== 'undefined' ? AppBridge.connectionListModel : null
                clip: true

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 52
                    color: index % 2 === 0 ? "transparent" : ThemeModel.hover
                    radius: 8

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 16

                        // 状态指示器
                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: connectionState === 2 ? ThemeModel.success
                                   : (connectionState === 1 ? ThemeModel.warning : ThemeModel.textMuted)
                        }

                        Text {
                            text: host; color: ThemeModel.text; font.pixelSize: 14
                            Layout.preferredWidth: 180; elide: Text.ElideRight
                        }
                        Text {
                            text: "● " + username; color: ThemeModel.textDim; font.pixelSize: 13
                            Layout.preferredWidth: 120; elide: Text.ElideRight
                        }
                        Text {
                            text: connectionState === 2 ? qsTr("Connected")
                                  : (connectionState === 1 ? qsTr("Connecting...") : qsTr("Disconnected"))
                            color: connectionState === 2 ? ThemeModel.success
                                   : (connectionState === 1 ? ThemeModel.warning : ThemeModel.textMuted)
                            font.pixelSize: 12
                        }

                        Item { Layout.fillWidth: true }

                        // 操作按钮
                        NeumorphismButton {
                            text: connectionState === 2 ? qsTr("Disconnect") : qsTr("Connect")
                            implicitWidth: 100; implicitHeight: 28
                            accent: connectionState !== 2
                            onClicked: {
                                if (connectionState === 2) {
                                    if (typeof AppBridge !== 'undefined')
                                        AppBridge.disconnect(host + "_" + username)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
