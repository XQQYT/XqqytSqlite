import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    property string currentTable: ""
    property int currentOffset: 0
    property int pageSize: 100

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // 左侧对象树 + 搜索
        ColumnLayout {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            spacing: 8

            // 搜索框
            NeumorphismTextField {
                id: searchField
                placeholderText: qsTr("Search tables...")
                Layout.fillWidth: true
            }

            // 对象树
            NeumorphismCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Text {
                        text: qsTr("Database Objects")
                        color: ThemeModel.text
                        font.pixelSize: 14
                        font.bold: true
                    }

                    TreeView {
                        id: treeView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: typeof AppBridge !== 'undefined'
                               ? AppBridge.databaseTreeModel : null
                        clip: true
                        columnWidthProvider: function(column) { return 260 }

                        delegate: TreeViewDelegate {
                            id: treeDelegate
                            anchors.leftMargin: 4

                            contentItem: RowLayout {
                                spacing: 4
                                Text {
                                    text: icon
                                    font.pixelSize: 13
                                }
                                Text {
                                    text: nodeName || ""
                                    color: ThemeModel.text
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.RightButton) {
                                        contextMenu.nodeType = nodeType
                                        contextMenu.nodeName = nodeName
                                        contextMenu.popup()
                                    } else if (nodeType === "table" || nodeType === "view") {
                                        page.currentTable = nodeName
                                        if (typeof AppBridge !== 'undefined') {
                                            AppBridge.queryTable(nodeName, 0, page.pageSize)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // 右侧数据表格
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // 标题行 + 搜索
            RowLayout {
                spacing: 12
                Text {
                    text: page.currentTable || qsTr("Table Data")
                    color: ThemeModel.text
                    font.pixelSize: 16
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                NeumorphismTextField {
                    id: dataSearch
                    placeholderText: qsTr("Search in data...")
                    implicitWidth: 200
                    onAccepted: {
                        if (typeof AppBridge !== 'undefined' && page.currentTable)
                            AppBridge.searchTable(page.currentTable, text)
                    }
                }
            }

            // 工具栏
            RowLayout {
                spacing: 8
                NeumorphismButton {
                    text: qsTr("Refresh"); implicitWidth: 90
                    onClicked: {
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.refreshCurrentTable()
                    }
                }
                NeumorphismButton {
                    text: qsTr("+ Row"); implicitWidth: 90
                    onClicked: insertDialog.open()
                }
                NeumorphismButton {
                    text: qsTr("Delete"); implicitWidth: 80
                    onClicked: {
                        confirmDialog.message = qsTr("Delete selected rows?")
                        confirmDialog.open()
                    }
                }
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 24; color: ThemeModel.border }
                NeumorphismButton {
                    text: qsTr("CSV"); implicitWidth: 60
                    onClicked: exportData("csv")
                }
                NeumorphismButton {
                    text: qsTr("JSON"); implicitWidth: 60
                    onClicked: exportData("json")
                }
                NeumorphismButton {
                    text: qsTr("SQL"); implicitWidth: 60
                    onClicked: exportData("sql")
                }
            }

            // 表格
            NeumorphismCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                TableView {
                    id: dataTable
                    anchors.fill: parent
                    anchors.margins: 4
                    model: typeof AppBridge !== 'undefined'
                           ? AppBridge.tableDataModel : null
                    clip: true
                    columnSpacing: 0
                    rowSpacing: 0
                    boundsBehavior: Flickable.StopAtBounds
                    selectionMode: TableView.ExtendedSelection

                    delegate: Rectangle {
                        id: cellDelegate
                        implicitWidth: 130
                        implicitHeight: 32
                        color: {
                            if (cellSelected) return ThemeModel.selected
                            if (row % 2 === 0) return "transparent"
                            return ThemeModel.hover
                        }
                        border.color: ThemeModel.border
                        border.width: 0.5

                        property bool cellSelected: false
                        property bool editing: false

                        Text {
                            anchors.fill: parent
                            anchors.margins: 6
                            text: modelData || ""
                            color: ThemeModel.text
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            visible: !cellDelegate.editing
                        }

                        // 双击编辑
                        TextInput {
                            anchors.fill: parent
                            anchors.margins: 4
                            text: modelData || ""
                            color: ThemeModel.text
                            font.pixelSize: 12
                            visible: cellDelegate.editing
                            focus: visible
                            selectByMouse: true
                            Keys.onPressed: (event) => {
                                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                    if (typeof AppBridge !== 'undefined' && page.currentTable) {
                                        AppBridge.updateCell(
                                            page.currentTable,
                                            "rowid = " + (tableData.row + 1),
                                            tableData.column, text
                                        )
                                    }
                                    cellDelegate.editing = false
                                } else if (event.key === Qt.Key_Escape) {
                                    cellDelegate.editing = false
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onDoubleClicked: {
                                cellDelegate.editing = true
                            }
                        }
                    }

                }
            }

            // 分页
            RowLayout {
                spacing: 12
                Layout.alignment: Qt.AlignRight

                NeumorphismButton {
                    text: qsTr("◀ Prev"); implicitWidth: 90; implicitHeight: 28
                    enabled: page.currentOffset > 0
                    onClicked: {
                        page.currentOffset = Math.max(0, page.currentOffset - page.pageSize)
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.queryTable(page.currentTable,
                                                 page.currentOffset, page.pageSize)
                    }
                }
                Text {
                    text: qsTr("Page %1").arg(Math.floor(page.currentOffset / page.pageSize) + 1)
                    color: ThemeModel.textDim; font.pixelSize: 12
                }
                NeumorphismButton {
                    text: qsTr("Next ▶"); implicitWidth: 90; implicitHeight: 28
                    onClicked: {
                        page.currentOffset += page.pageSize
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.queryTable(page.currentTable,
                                                 page.currentOffset, page.pageSize)
                    }
                }
            }
        }
    }

    // 右键菜单
    Menu {
        id: contextMenu
        property string nodeType: ""
        property string nodeName: ""

        MenuItem {
            text: qsTr("View Structure")
            onClicked: {
                if (typeof AppBridge !== 'undefined')
                    AppBridge.filterColumn(contextMenu.nodeName, "__structure__", "")
            }
        }
        MenuItem {
            text: qsTr("Export DDL")
            onClicked: {
                if (typeof AppBridge !== 'undefined')
                    AppBridge.exportData(contextMenu.nodeName, "sql",
                                         "/tmp/" + contextMenu.nodeName + ".sql")
            }
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Rename")
            enabled: contextMenu.nodeType === "table"
            onClicked: renameDialog.open()
        }
        MenuItem {
            text: qsTr("Delete")
            enabled: contextMenu.nodeType === "table" || contextMenu.nodeType === "view"
            onClicked: {
                confirmDialog.message = qsTr("Drop %1 \"%2\"?").arg(contextMenu.nodeType).arg(contextMenu.nodeName)
                confirmDialog.accepted.connect(function() {
                    if (typeof AppBridge !== 'undefined')
                        AppBridge.dropTable(contextMenu.nodeName)
                })
                confirmDialog.open()
            }
        }
    }

    // 重命名对话框
    Dialog {
        id: renameDialog
        title: qsTr("Rename Table")
        anchors.centerIn: parent; modal: true
        background: Rectangle {
            radius: ThemeModel.cardRadius
            color: ThemeModel.panel
            border.color: ThemeModel.border
        }
        contentItem: ColumnLayout {
            spacing: 12; width: 300
            Text { text: qsTr("New name:"); color: ThemeModel.text }
            NeumorphismTextField { id: renameField; Layout.fillWidth: true }
            RowLayout {
                spacing: 12; Layout.alignment: Qt.AlignRight
                NeumorphismButton { text: qsTr("Cancel"); onClicked: renameDialog.close() }
                NeumorphismButton {
                    text: qsTr("Rename"); accent: true
                    onClicked: {
                        if (typeof AppBridge !== 'undefined')
                            AppBridge.renameTable(contextMenu.nodeName, renameField.text)
                        renameDialog.close()
                    }
                }
            }
        }
    }

    // 插入行对话框
    Dialog {
        id: insertDialog
        title: qsTr("Insert Row")
        anchors.centerIn: parent; modal: true
        background: Rectangle {
            radius: ThemeModel.cardRadius
            color: ThemeModel.panel; border.color: ThemeModel.border
        }
        contentItem: ColumnLayout {
            spacing: 12; width: 320
            Text { text: qsTr("Enter values (comma-separated):"); color: ThemeModel.textDim; font.pixelSize: 12 }
            NeumorphismTextField { id: insertField; placeholderText: qsTr("value1, value2, ..."); Layout.fillWidth: true }
            RowLayout {
                spacing: 12; Layout.alignment: Qt.AlignRight
                NeumorphismButton { text: qsTr("Cancel"); onClicked: insertDialog.close() }
                NeumorphismButton {
                    text: qsTr("Insert"); accent: true
                    onClicked: {
                        if (typeof AppBridge !== 'undefined' && page.currentTable) {
                            var values = insertField.text.split(",")
                            AppBridge.insertRow(page.currentTable, [], values)
                        }
                        insertDialog.close()
                    }
                }
            }
        }
    }

    // 数据导出辅助函数
    function exportData(format) {
        if (typeof AppBridge !== 'undefined' && page.currentTable) {
            var path = "/tmp/" + page.currentTable + "." + format
            AppBridge.exportData(page.currentTable, format, path)
        }
    }

    // 连接表加载信号
    Connections {
        target: typeof AppBridge !== 'undefined' ? AppBridge : null
        enabled: typeof AppBridge !== 'undefined'
        function onTablesLoaded() {
            // 树自动刷新
        }
        function onTableDataReady() {
            // 数据表格通过 model 自动刷新
        }
    }
}
