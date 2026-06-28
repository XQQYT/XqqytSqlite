import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    property var designerModel: typeof AppBridge !== 'undefined'
                                 ? AppBridge.tableDesignerModel : null

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // === 左侧：列定义编辑器 ===
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 8

            // 表名 + WITHOUT ROWID
            RowLayout {
                spacing: 12
                Text { text: qsTr("Table:"); color: ThemeModel.text; font.pixelSize: 14 }
                NeumorphismTextField {
                    id: tableNameField
                    text: designerModel ? designerModel.tableName : ""
                    implicitWidth: 200
                    onTextChanged: { if (designerModel) designerModel.tableName = text }
                }
                CheckBox {
                    text: qsTr("WITHOUT ROWID")
                    checked: designerModel ? designerModel.withoutRowid : false
                    contentItem: Text { text: parent.text; color: ThemeModel.textDim; font.pixelSize: 12 }
                    onClicked: { if (designerModel) designerModel.withoutRowid = checked }
                }
            }

            // 工具栏
            RowLayout {
                spacing: 8
                NeumorphismButton {
                    text: qsTr("+ Add Column"); accent: true; implicitWidth: 130
                    onClicked: { if (designerModel) designerModel.addColumn() }
                }
                NeumorphismButton {
                    text: qsTr("+ Index"); implicitWidth: 100
                    onClicked: indexDialog.open()
                }
                NeumorphismButton {
                    text: qsTr("+ Foreign Key"); implicitWidth: 130
                    onClicked: fkDialog.open()
                }
                Item { Layout.fillWidth: true }
                NeumorphismButton {
                    text: qsTr("Clear All"); implicitWidth: 100
                    onClicked: {
                        confirmDialog.message = qsTr("Clear all column definitions?")
                        confirmDialog.accepted.connect(function() {
                            if (designerModel) designerModel.clear()
                        })
                        confirmDialog.open()
                    }
                }
            }

            // 列定义表格
            NeumorphismCard {
                Layout.fillWidth: true; Layout.fillHeight: true

                ScrollView {
                    anchors.fill: parent; anchors.margins: 8; clip: true

                    ColumnLayout {
                        spacing: 4; width: parent.width - 16

                        // 表头
                        RowLayout {
                            spacing: 4
                            HeaderCell { text: qsTr("Name"); w: 130 }
                            HeaderCell { text: qsTr("Type"); w: 90 }
                            HeaderCell { text: qsTr("PK"); w: 36 }
                            HeaderCell { text: qsTr("AI"); w: 36 }
                            HeaderCell { text: qsTr("NOT NULL"); w: 70 }
                            HeaderCell { text: qsTr("UNIQUE"); w: 60 }
                            HeaderCell { text: qsTr("Default"); w: 90 }
                            Item { Layout.fillWidth: true }
                        }

                        // 列行
                        Repeater {
                            model: designerModel

                            Rectangle {
                                Layout.fillWidth: true
                                height: 40; radius: 8
                                color: index % 2 === 0 ? "transparent" : ThemeModel.hover

                                RowLayout {
                                    anchors.fill: parent; anchors.margins: 4; spacing: 4

                                    NeumorphismTextField {
                                        text: colName; implicitWidth: 130; fieldRadius: 8
                                        onTextChanged: { if (designerModel) designerModel.setData(
                                            designerModel.index(index, 0), text, TableDesignerModel.NameRole) }
                                    }
                                    ComboBox {
                                        currentIndex: typeIndex(colType)
                                        model: ["TEXT", "INTEGER", "REAL", "BLOB", "NUMERIC"]
                                        implicitWidth: 90
                                        background: Rectangle {
                                            radius: 8; color: ThemeModel.panel; border.color: ThemeModel.border
                                        }
                                        contentItem: Text {
                                            text: currentText; color: ThemeModel.text; font.pixelSize: 12
                                            verticalAlignment: Text.AlignVCenter; leftPadding: 6
                                        }
                                        onCurrentTextChanged: {
                                            if (designerModel) designerModel.setData(
                                                designerModel.index(index, 0), currentText, TableDesignerModel.TypeRole)
                                        }
                                    }
                                    CheckBox {
                                        checked: primaryKey; implicitWidth: 30
                                        onClicked: { if (designerModel) designerModel.setData(
                                            designerModel.index(index, 0), checked, TableDesignerModel.PrimaryKeyRole) }
                                    }
                                    CheckBox {
                                        checked: autoIncrement; implicitWidth: 30; enabled: primaryKey && colType === "INTEGER"
                                        onClicked: { if (designerModel) designerModel.setData(
                                            designerModel.index(index, 0), checked, TableDesignerModel.AutoIncrementRole) }
                                    }
                                    CheckBox {
                                        checked: notNull; implicitWidth: 60
                                        onClicked: { if (designerModel) designerModel.setData(
                                            designerModel.index(index, 0), checked, TableDesignerModel.NotNullRole) }
                                    }
                                    CheckBox {
                                        checked: unique; implicitWidth: 50
                                        onClicked: { if (designerModel) designerModel.setData(
                                            designerModel.index(index, 0), checked, TableDesignerModel.UniqueRole) }
                                    }
                                    NeumorphismTextField {
                                        text: defaultValue; implicitWidth: 90; fieldRadius: 8
                                        onTextChanged: { if (designerModel) designerModel.setData(
                                            designerModel.index(index, 0), text, TableDesignerModel.DefaultValueRole) }
                                    }
                                    NeumorphismButton {
                                        text: "×"; implicitWidth: 28; implicitHeight: 28
                                        onClicked: { if (designerModel) designerModel.removeColumn(index) }
                                    }
                                }
                            }
                        }

                        // 索引列表
                        Text {
                            text: designerModel && designerModel.indexCount > 0
                                  ? qsTr("Indexes (%1):").arg(designerModel.indexCount) : ""
                            color: ThemeModel.textDim; font.pixelSize: 12; visible: text !== ""
                            Layout.topMargin: 8
                        }
                        Repeater {
                            model: designerModel ? designerModel.indexCount : 0
                            Rectangle {
                                Layout.fillWidth: true; height: 28; radius: 6
                                color: ThemeModel.hover
                                RowLayout {
                                    anchors.fill: parent; anchors.margins: 6
                                    property var idx: designerModel ? designerModel.getIndex(index) : null
                                    Text {
                                        text: (parent.idx ? (parent.idx["unique"] ? "UNIQUE " : "") + parent.idx["name"] : "")
                                              + "(" + (parent.idx ? parent.idx["columns"].join(", ") : "") + ")"
                                        color: ThemeModel.textDim; font.pixelSize: 12; Layout.fillWidth: true
                                    }
                                    NeumorphismButton {
                                        text: "×"; implicitWidth: 24; implicitHeight: 24
                                        onClicked: { if (designerModel) designerModel.removeIndex(index) }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // === 右侧：DDL 预览 + 执行 ===
        ColumnLayout {
            Layout.preferredWidth: 360; Layout.fillHeight: true; spacing: 8

            RowLayout {
                spacing: 12
                Text { text: qsTr("DDL Preview"); color: ThemeModel.text; font.pixelSize: 14; font.bold: true }
                Item { Layout.fillWidth: true }
                NeumorphismButton {
                    text: qsTr("Refresh"); implicitWidth: 80
                    onClicked: {
                        ddlPreview.text = typeof AppBridge !== 'undefined'
                            ? AppBridge.generateCreateTableDDL() : ""
                    }
                }
            }

            NeumorphismCard {
                Layout.fillWidth: true; Layout.fillHeight: true
                inset: true

                Flickable {
                    anchors.fill: parent; anchors.margins: 12
                    contentHeight: ddlPreview.implicitHeight
                    clip: true

                    Text {
                        id: ddlPreview
                        width: parent.width
                        text: typeof AppBridge !== 'undefined'
                              ? AppBridge.generateCreateTableDDL() : ""
                        color: ThemeModel.text
                        font.family: "Consolas, 'Courier New', monospace"
                        font.pixelSize: 13
                        wrapMode: Text.Wrap
                        textFormat: Text.PlainText
                    }
                }
            }

            // 执行按钮
            NeumorphismButton {
                text: qsTr("▶ Create Table on Device")
                accent: true; implicitWidth: Layout.fillWidth; implicitHeight: 44
                Layout.fillWidth: true
                onClicked: {
                    confirmDialog.message = qsTr("Create table \"%1\" on the remote database?")
                        .arg(designerModel ? designerModel.tableName : "")
                    confirmDialog.accepted.connect(function() {
                        if (typeof AppBridge !== 'undefined') {
                            AppBridge.executeCreateTable("current", "/remote/path.db")
                            _toast.show(qsTr("Table created!"), "success")
                        }
                    })
                    confirmDialog.open()
                }
            }
        }
    }

    // 表头组件
    component HeaderCell: Text {
        property int w: 80
        Layout.preferredWidth: w
        text: parent.text
        color: ThemeModel.textDim; font.pixelSize: 11; font.bold: true
        elide: Text.ElideRight
    }

    // 类型索引辅助
    function typeIndex(type) {
        var types = ["TEXT", "INTEGER", "REAL", "BLOB", "NUMERIC"]
        var idx = types.indexOf(type.toUpperCase())
        return idx >= 0 ? idx : 0
    }

    // ===== 索引对话框 =====
    Dialog {
        id: indexDialog; title: qsTr("Add Index")
        anchors.centerIn: parent; modal: true
        background: Rectangle { radius: ThemeModel.cardRadius; color: ThemeModel.panel; border.color: ThemeModel.border }
        contentItem: ColumnLayout {
            spacing: 12; width: 320
            Text { text: qsTr("Index name:"); color: ThemeModel.text; font.pixelSize: 13 }
            NeumorphismTextField { id: idxName; Layout.fillWidth: true }
            Text { text: qsTr("Columns (comma-separated):"); color: ThemeModel.text; font.pixelSize: 13 }
            NeumorphismTextField { id: idxCols; Layout.fillWidth: true; placeholderText: "col1, col2" }
            CheckBox { id: idxUnique; text: qsTr("UNIQUE") }
            RowLayout { spacing: 12; Layout.alignment: Qt.AlignRight
                NeumorphismButton { text: qsTr("Cancel"); onClicked: indexDialog.close() }
                NeumorphismButton {
                    text: qsTr("Add"); accent: true
                    onClicked: {
                        if (designerModel) {
                            var cols = idxCols.text.split(",").map(function(c) { return c.trim() }).filter(Boolean)
                            designerModel.addIndex(idxName.text || "idx_auto", cols, idxUnique.checked)
                        }
                        indexDialog.close()
                    }
                }
            }
        }
    }

    // ===== 外键对话框 =====
    Dialog {
        id: fkDialog; title: qsTr("Add Foreign Key")
        anchors.centerIn: parent; modal: true
        background: Rectangle { radius: ThemeModel.cardRadius; color: ThemeModel.panel; border.color: ThemeModel.border }
        contentItem: ColumnLayout {
            spacing: 10; width: 340
            GridLayout {
                columns: 2; rowSpacing: 8; columnSpacing: 12; Layout.fillWidth: true
                Text { text: qsTr("Column:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: fkCol; Layout.fillWidth: true }
                Text { text: qsTr("Ref Table:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: fkRefTable; Layout.fillWidth: true }
                Text { text: qsTr("Ref Column:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                NeumorphismTextField { id: fkRefCol; Layout.fillWidth: true }
                Text { text: qsTr("ON DELETE:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                ComboBox {
                    id: fkOnDelete; model: ["NO ACTION", "CASCADE", "SET NULL", "SET DEFAULT", "RESTRICT"]
                    background: Rectangle { radius: 8; color: ThemeModel.panel; border.color: ThemeModel.border }
                }
                Text { text: qsTr("ON UPDATE:"); color: ThemeModel.textDim; font.pixelSize: 13 }
                ComboBox {
                    id: fkOnUpdate; model: ["NO ACTION", "CASCADE", "SET NULL", "SET DEFAULT", "RESTRICT"]
                    background: Rectangle { radius: 8; color: ThemeModel.panel; border.color: ThemeModel.border }
                }
            }
            RowLayout { spacing: 12; Layout.alignment: Qt.AlignRight
                NeumorphismButton { text: qsTr("Cancel"); onClicked: fkDialog.close() }
                NeumorphismButton {
                    text: qsTr("Add"); accent: true
                    onClicked: {
                        if (designerModel) designerModel.addForeignKey(
                            fkCol.text, fkRefTable.text, fkRefCol.text,
                            fkOnDelete.currentText, fkOnUpdate.currentText)
                        fkDialog.close()
                    }
                }
            }
        }
    }
}
