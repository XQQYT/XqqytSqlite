import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    title: "XqqytSqlite"
    color: ThemeModel.bg

    // 主题切换动画
    Behavior on color {
        ColorAnimation { duration: 400; easing.type: Easing.InOutCubic }
    }

    // 全局 Toast 引用
    property alias toast: _toast
    property alias loadingOverlay: _loading

    // 当前页面索引
    property int currentPageIndex: 0

    // 主布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 菜单栏
        AppMenuBar {
            Layout.fillWidth: true
        }

        // 内容区
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // 左侧导航
            SideNav {
                id: sideNav
                Layout.preferredWidth: 64
                Layout.fillHeight: true
                onPageSelected: (index) => {
                    currentPageIndex = index
                    if (index < 0) {
                        while (pageStack.depth > 1) pageStack.pop()
                        return
                    }
                    if (pageStack.depth > 1) pageStack.pop()
                    pageStack.push(pages[index])
                }
            }

            // 主区域
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // 标签页（按需显示）
                TabBar {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                }

                // 页面栈（淡入淡出切换）
                StackView {
                    id: pageStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    initialItem: welcomePage

                    pushEnter: Transition {
                        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.OutCubic }
                    }
                    pushExit: Transition {
                        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.InCubic }
                    }
                    popEnter: Transition {
                        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 300; easing.type: Easing.OutCubic }
                    }
                    popExit: Transition {
                        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.InCubic }
                    }
                }

                // 结果面板
                Rectangle {
                    id: resultPanel
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    color: ThemeModel.panel
                    border.color: ThemeModel.border

                    visible: false

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 4

                        // 三个标签：结果 / 消息 / 日志
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Row {
                                spacing: 0
                                Repeater {
                                    model: [qsTr("Results"), qsTr("Messages"), qsTr("Log")]
                                    NeumorphismButton {
                                        text: modelData
                                        accent: index === 0
                                        implicitWidth: 80
                                        implicitHeight: 28
                                        btnRadius: 8
                                    }
                                }
                            }

                            // 结果表格占位
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 12
                                color: Qt.darker(ThemeModel.panel, 1.05)
                                border.color: ThemeModel.border

                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("Query results will appear here")
                                    color: ThemeModel.textMuted
                                    font.pixelSize: 13
                                }
                            }
                        }
                    }
                }
            }
        }

        // 状态栏
        StatusBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
        }
    }

    // 页面组件列表
    readonly property var pages: [
        sftpPage,
        dataBrowsePage,
        sqlEditorPage,
        tableDesignerPage,
        settingsPage
    ]

    // 主页 — 设备连接管理（参考 Remote-sqlite ConnectionPage）
    Component {
        id: welcomePage
        Rectangle {
            color: ThemeModel.bg
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 24; spacing: 16

                // 标题栏
                RowLayout {
                    Text { text: qsTr("SSH Connections"); color: ThemeModel.text; font.pixelSize: 22; font.bold: true; Layout.fillWidth: true }
                    NeumorphismButton {
                        text: qsTr("+ New Connection"); accent: true; implicitWidth: 160; implicitHeight: 40
                        onClicked: connectionDialog.open()
                    }
                }

                // 设备列表
                ListView {
                    id: connList2
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: typeof AppBridge !== 'undefined' ? AppBridge.connectionListModel : null
                    clip: true; spacing: 10
                    delegate: NeumorphismCard {
                        width: ListView.view.width - 4; implicitHeight: 120
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 16; spacing: 8
                            RowLayout {
                                spacing: 12
                                Rectangle { width: 10; height: 10; radius: 5
                                    color: connectionState === 2 ? ThemeModel.success
                                           : (connectionState === 1 ? ThemeModel.warning : ThemeModel.textMuted) }
                                Text { text: host; color: ThemeModel.text; font.pixelSize: 16; font.bold: true }
                                Item { Layout.fillWidth: true }
                                NeumorphismButton {
                                    text: connectionState === 2 ? qsTr("Disconnect") : qsTr("Connect")
                                    implicitWidth: 100; implicitHeight: 34
                                    accent: connectionState !== 2
                                    onClicked: {
                                        if (typeof AppBridge !== 'undefined') {
                                            if (connectionState === 2) AppBridge.disconnect(host + "_" + username)
                                            else AppBridge.connectToHost(host, username, "", port)
                                        }
                                    }
                                }
                            }
                            Text { text: username + "  ·  port " + port; color: ThemeModel.textDim; font.pixelSize: 13 }
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: ThemeModel.border }
                            Text {
                                text: connectionState === 2 ? qsTr("Connected — click Database tab to browse")
                                     : qsTr("Click Connect to establish SSH connection")
                                color: ThemeModel.textMuted; font.pixelSize: 12
                            }
                        }
                    }

                    Text {
                        text: qsTr("No saved connections. Click '+ New Connection' to add a device.")
                        color: ThemeModel.textDim; font.pixelSize: 14; Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 40; visible: connList2.count === 0
                    }
                }
            }
        }
    }

    // SFTP 管理页面
    Component {
        id: sftpPage
        SftpManagerPage {}
    }

    // 数据浏览页面
    Component {
        id: dataBrowsePage
        DataBrowsePage {}
    }

    // SQL 编辑器页面
    Component {
        id: sqlEditorPage
        SqlEditorPage {}
    }

    Component {
        id: tableDesignerPage
        TableDesignerPage {}
    }
    Component {
        id: settingsPage
        SettingsPage {}
    }

    // SSH 连接弹窗
    Dialog {
        id: connectionDialog
        title: qsTr("New SSH Connection")
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        width: 400

        // 关闭按钮
        header: RowLayout {
            Text {
                text: qsTr("New SSH Connection")
                color: ThemeModel.text; font.pixelSize: 16; font.bold: true
                Layout.fillWidth: true; Layout.leftMargin: 16
            }
            NeumorphismButton {
                text: "✕"; implicitWidth: 36; implicitHeight: 36
                onClicked: connectionDialog.close()
            }
        }

        background: Rectangle {
            radius: ThemeModel.cardRadius
            color: ThemeModel.panel; border.color: ThemeModel.border
        }

        contentItem: ColumnLayout {
            spacing: 10; width: 360

            GridLayout {
                columns: 2; rowSpacing: 8; columnSpacing: 12; Layout.fillWidth: true
                Text { text: qsTr("Host:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: hostField; placeholderText: qsTr("hostname or IP"); Layout.fillWidth: true }
                Text { text: qsTr("Port:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: portField; text: "22"; implicitWidth: 80 }
                Text { text: qsTr("User:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: userField; placeholderText: qsTr("username"); Layout.fillWidth: true }
                Text { text: qsTr("Pass:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: passField; placeholderText: qsTr("password"); echoMode: TextInput.Password; Layout.fillWidth: true }
            }

            Item { Layout.preferredHeight: 4 }

            RowLayout {
                spacing: 12; Layout.alignment: Qt.AlignRight
                NeumorphismButton {
                    text: qsTr("Cancel"); implicitWidth: 90
                    onClicked: connectionDialog.close()
                }
                NeumorphismButton {
                    text: qsTr("Test"); implicitWidth: 70
                    onClicked: {
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.testConnection(hostField.text, userField.text, passField.text, parseInt(portField.text) || 22)
                    }
                }
                NeumorphismButton {
                    text: qsTr("Connect"); accent: true; implicitWidth: 100
                    onClicked: {
                        if (typeof AppBridge !== 'undefined') {
                            var id = AppBridge.connectToHost(hostField.text, userField.text, passField.text, parseInt(portField.text) || 22)
                            if (id) { connectionDialog.close(); _toast.show(qsTr("Connected: ") + hostField.text, "success") }
                        }
                    }
                }
            }
        }
    }

    // 加载遮罩
    LoadingOverlay {
        id: _loading
    }

    // Toast 提示
    Toast {
        id: _toast
    }

    // 确认弹窗
    ConfirmDialog {
        id: confirmDialog
    }

    // 启动画面（1 秒后自动淡出）
    SplashScreen {
        id: splashScreen
        Component.onCompleted: {
            splashScreen.progress = 0.5
            splashScreen.progress = 1.0
        }
    }

    AboutDialog { id: aboutDialog }
}
