import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // === 左侧：远程文件浏览器 ===
        ColumnLayout {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: qsTr("Remote Files (SFTP)")
                color: ThemeModel.text; font.pixelSize: 14; font.bold: true
            }

            // 路径导航
            RowLayout {
                spacing: 4
                NeumorphismButton {
                    text: "↩"; implicitWidth: 36; implicitHeight: 36
                    onClicked: {
                        var p = pathField.text
                        if (p === "/") return
                        var parts = p.split("/").filter(Boolean)
                        parts.pop()
                        pathField.text = parts.length ? "/" + parts.join("/") : "/"
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.sftpListDirectory(pathField.text)
                    }
                }
                NeumorphismTextField {
                    id: pathField; text: "/"; Layout.fillWidth: true
                    onAccepted: {
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.sftpListDirectory(text)
                    }
                }
                NeumorphismButton {
                    text: "→"; implicitWidth: 40; implicitHeight: 36
                    onClicked: {
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.sftpListDirectory(pathField.text)
                    }
                }
            }

            // 文件列表
            NeumorphismCard {
                Layout.fillWidth: true; Layout.fillHeight: true

                ListView {
                    id: fileList
                    anchors.fill: parent; anchors.margins: 4
                    model: typeof AppBridge !== 'undefined'
                           ? AppBridge.sftpFileModel : null
                    clip: true

                    delegate: Rectangle {
                        width: ListView.view.width; height: 40
                        color: index % 2 === 0 ? "transparent" : ThemeModel.hover
                        radius: 6

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 8; spacing: 8
                            Text {
                                text: isDirectory ? "📁" : (fileName.endsWith(".db") || fileName.endsWith(".sqlite") ? "🗄️" : "📄")
                                font.pixelSize: 16
                                Layout.preferredWidth: 24
                            }
                            Text {
                                text: fileName
                                color: ThemeModel.text; font.pixelSize: 13
                                Layout.fillWidth: true; elide: Text.ElideRight
                            }
                            Text {
                                text: isDirectory ? "" : formatSize(fileSize)
                                color: ThemeModel.textMuted; font.pixelSize: 11
                                visible: !isDirectory
                            }
                            Text {
                                text: isDirectory ? "" : mtime.substring(0, 10)
                                color: ThemeModel.textMuted; font.pixelSize: 10
                                visible: !isDirectory
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            cursorShape: Qt.PointingHandCursor
                            onClicked: (mouse) => {
                                if (mouse.button === Qt.RightButton) {
                                    fileMenu.fileName = fileName
                                    fileMenu.remotePath = pathField.text.endsWith("/")
                                        ? pathField.text + fileName
                                        : pathField.text + "/" + fileName
                                    fileMenu.isDir = isDirectory
                                    fileMenu.popup()
                                } else if (isDirectory) {
                                    var newPath = pathField.text
                                    if (newPath !== "/") newPath += "/"
                                    newPath += fileName
                                    pathField.text = newPath
                                    if (typeof AppBridge !== 'undefined')
                                        AppBridge.sftpListDirectory(newPath)
                                }
                            }
                        }
                    }
                }
            }
        }

        // === 右侧：同步状态 + 操作面板 ===
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 12

            Text {
                text: qsTr("Sync Status")
                color: ThemeModel.text; font.pixelSize: 14; font.bold: true
            }

            // 同步列表
            NeumorphismCard {
                Layout.fillWidth: true; Layout.fillHeight: true

                ListView {
                    anchors.fill: parent; anchors.margins: 8
                    model: typeof AppBridge !== 'undefined'
                           ? AppBridge.syncStatusModel : null
                    clip: true; spacing: 6

                    delegate: Rectangle {
                        width: ListView.view.width; height: 72
                        radius: 10
                        color: ThemeModel.panel
                        border.width: 1
                        border.color: syncState === 5 ? ThemeModel.error  // Conflict
                                    : (syncState === 3 ? ThemeModel.accent2 // Dirty
                                    : (syncState === 1 || syncState === 4 // Syncing/Uploading
                                       ? ThemeModel.accent : ThemeModel.border))

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 12

                            // 状态指示
                            Rectangle {
                                width: 40; height: 40; radius: 20
                                color: syncState === 6 ? ThemeModel.success
                                     : (syncState === 5 ? ThemeModel.error
                                     : (syncState === 3 ? ThemeModel.warning
                                     : (syncState <= 1 ? ThemeModel.info
                                     : ThemeModel.accent)))
                                Text {
                                    anchors.centerIn: parent
                                    text: stateIcon(syncState)
                                    font.pixelSize: 20
                                }
                            }

                            // 信息
                            ColumnLayout {
                                spacing: 2
                                Text {
                                    text: dbName; color: ThemeModel.text
                                    font.pixelSize: 14; font.bold: true
                                }
                                Text {
                                    text: stateText(syncState)
                                    color: ThemeModel.textDim; font.pixelSize: 12
                                }

                                // 进度条（传输中）
                                ProgressBar {
                                    Layout.fillWidth: true
                                    visible: syncState === 1 || syncState === 4
                                    value: transferProgress / 100.0
                                    height: 4
                                }
                            }

                            Item { Layout.fillWidth: true }

                            // 操作按钮
                            ColumnLayout {
                                spacing: 4
                                visible: syncState !== 6  // 非同步完成状态显示按钮

                                // 同步按钮（本地有修改时）
                                NeumorphismButton {
                                    text: qsTr("Upload")
                                    visible: syncState === 3 || syncState === 5
                                    accent: true; implicitWidth: 80; implicitHeight: 28
                                    onClicked: {
                                        if (typeof AppBridge !== 'undefined')
                                            AppBridge.syncUpload(remotePath, 0)
                                        _toast.show(qsTr("Uploading..."), "success")
                                    }
                                }

                                // 下载按钮
                                NeumorphismButton {
                                    text: qsTr("Download")
                                    visible: syncState === 0
                                    implicitWidth: 80; implicitHeight: 28
                                    onClicked: {
                                        if (typeof AppBridge !== 'undefined')
                                            AppBridge.syncDownloadForEdit(remotePath)
                                        _toast.show(qsTr("Downloading..."), "success")
                                    }
                                }

                                // 放弃按钮
                                NeumorphismButton {
                                    text: qsTr("Discard")
                                    visible: syncState === 2 || syncState === 3 || syncState === 5
                                    implicitWidth: 80; implicitHeight: 28
                                    onClicked: {
                                        confirmDialog.message = qsTr("Discard local changes to %1?").arg(dbName)
                                        confirmDialog.accepted.connect(function() {
                                            if (typeof AppBridge !== 'undefined')
                                                AppBridge.syncDiscard(remotePath)
                                        })
                                        confirmDialog.open()
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 快速操作区
            RowLayout {
                spacing: 8
                NeumorphismButton {
                    text: qsTr("Refresh File List"); implicitWidth: 150
                    onClicked: {
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.sftpListDirectory(pathField.text)
                    }
                }
                NeumorphismButton {
                    text: qsTr("New Directory"); implicitWidth: 130
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: typeof AppBridge !== 'undefined'
                          ? qsTr("%1 active").arg(AppBridge.syncStatusModel.activeCount)
                          : "0 active"
                    color: ThemeModel.textDim; font.pixelSize: 12
                }
            }
        }
    }

    // 文件右键菜单
    Menu {
        id: fileMenu
        property string fileName: ""
        property string remotePath: ""
        property bool isDir: false

        MenuItem {
            text: qsTr("Download for Editing")
            visible: fileMenu.fileName.endsWith(".db") || fileMenu.fileName.endsWith(".sqlite")
            onClicked: {
                if (typeof AppBridge !== 'undefined')
                    AppBridge.syncDownloadForEdit(fileMenu.remotePath)
                _toast.show(qsTr("Downloading %1...").arg(fileMenu.fileName), "success")
            }
        }
        MenuItem {
            text: qsTr("Download to...")
            onClicked: {
                if (typeof AppBridge !== 'undefined')
                    AppBridge.sftpDownload(fileMenu.remotePath, "/tmp/" + fileMenu.fileName)
                _toast.show(qsTr("Downloading %1...").arg(fileMenu.fileName), "success")
            }
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Rename")
        }
        MenuItem {
            text: qsTr("Delete")
            onClicked: {
                confirmDialog.message = qsTr("Delete %1 from remote?").arg(fileMenu.fileName)
                confirmDialog.open()
            }
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("File Info")
            onClicked: {
                _toast.show(
                    qsTr("Name: %1\nPath: %2").arg(fileMenu.fileName).arg(fileMenu.remotePath),
                    "success")
            }
        }
    }

    // ===== 辅助函数 =====

    function formatSize(bytes) {
        if (bytes < 1024) return bytes + " B"
        if (bytes < 1048576) return (bytes / 1024).toFixed(1) + " KB"
        return (bytes / 1048576).toFixed(1) + " MB"
    }

    function stateIcon(state) {
        switch (state) {
        case 0: return "⬇";   // RemoteOnly
        case 1: return "📥";  // Syncing
        case 2: return "📝";  // LocalEditing
        case 3: return "📤";  // LocalDirty
        case 4: return "⬆";   // Uploading
        case 5: return "⚠";   // Conflict
        case 6: return "✅";  // InSync
        default: return "❓";
        }
    }

    function stateText(state) {
        switch (state) {
        case 0: return qsTr("Remote only — click to download")
        case 1: return qsTr("Downloading...")
        case 2: return qsTr("Editing locally")
        case 3: return qsTr("Modified — needs upload")
        case 4: return qsTr("Uploading...")
        case 5: return qsTr("Conflict detected!")
        case 6: return qsTr("In sync ✓")
        default: return qsTr("Unknown")
        }
    }
}
