import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: editorPage

    property var activeTab: null
    property var tabs: []

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // 左侧：片段面板
        Rectangle {
            id: snippetPanel
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            radius: ThemeModel.cardRadius
            color: ThemeModel.panel
            border.color: ThemeModel.border
            visible: false

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Text {
                        text: qsTr("SQL Snippets")
                        color: ThemeModel.text; font.pixelSize: 14; font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    NeumorphismButton {
                        text: qsTr("+"); implicitWidth: 32; implicitHeight: 28
                        onClicked: snippetAddDialog.open()
                    }
                }

                NeumorphismTextField {
                    id: snippetSearch
                    placeholderText: qsTr("Search snippets...")
                    Layout.fillWidth: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: typeof AppBridge !== 'undefined'
                           ? AppBridge.sqlSnippetModel : null
                    clip: true
                    spacing: 4

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 48
                        color: mouseArea.containsMouse ? ThemeModel.hover : "transparent"
                        radius: 6

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 2

                            Text {
                                text: snippetName || ""
                                color: ThemeModel.text; font.pixelSize: 13; font.bold: true
                                elide: Text.ElideRight; Layout.fillWidth: true
                            }
                            Text {
                                text: description || ""
                                color: ThemeModel.textMuted; font.pixelSize: 11
                                elide: Text.ElideRight; Layout.fillWidth: true
                            }
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (editorPage.activeTab)
                                    editorPage.activeTab.insertSql(sql || "")
                            }
                        }
                    }
                }
            }
        }

        // 右侧：编辑器主体
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // 标题 + 标签栏
            RowLayout {
                spacing: 8
                Text {
                    text: qsTr("SQL Editor")
                    color: ThemeModel.text; font.pixelSize: 18; font.bold: true
                }
                Item { Layout.fillWidth: true }
                NeumorphismButton {
                    text: qsTr("Snippets"); implicitWidth: 100
                    onClicked: snippetPanel.visible = !snippetPanel.visible
                }
                NeumorphismButton {
                    text: qsTr("+ Tab"); implicitWidth: 80
                    onClicked: addTab()
                }
            }

            // 标签页行
            Row {
                id: tabRow
                spacing: 2
                Layout.fillWidth: true
            }

            // 工具栏
            RowLayout {
                spacing: 8
                NeumorphismButton {
                    text: qsTr("▶ Run (Ctrl+Enter)"); accent: true; implicitWidth: 160
                    onClicked: executeCurrentSql()
                }
                NeumorphismButton {
                    text: qsTr("Format SQL"); implicitWidth: 120
                    onClicked: formatCurrentSql()
                }
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 24; color: ThemeModel.border }
                NeumorphismButton {
                    text: qsTr("History"); implicitWidth: 100
                    onClicked: historyPanel.visible = !historyPanel.visible
                }
                NeumorphismButton {
                    text: qsTr("Explain"); implicitWidth: 100
                    onClicked: explainCurrentSql()
                }
                NeumorphismButton {
                    text: qsTr("Clear"); implicitWidth: 80
                    onClicked: { if (activeTab) activeTab.clear() }
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: activeTab
                          ? qsTr("Ln %1 Col %2").arg(
                              Math.max(1, activeTab.cursorLine + 1)).arg(
                              Math.max(1, activeTab.cursorColumn + 1))
                          : ""
                    color: ThemeModel.textDim; font.pixelSize: 11
                }
            }

            // 编辑区
            NeumorphismCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                inset: true

                StackView {
                    id: editorStack
                    anchors.fill: parent
                    anchors.margins: 2
                    clip: true
                    initialItem: editorComponent
                }
            }

            // 自动补全弹窗
            Popup {
                id: autoCompletePopup
                y: activeTab ? activeTab.cursorY : 0
                width: 260; height: 200
                padding: 4
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

                background: Rectangle {
                    radius: 12; color: ThemeModel.panel
                    border.color: ThemeModel.border
                }

                ListView {
                    anchors.fill: parent
                    clip: true
                    model: autoCompleteItems
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 28; radius: 6
                        color: autoMouse.containsMouse ? ThemeModel.hover : "transparent"
                        Text {
                            anchors.fill: parent; anchors.margins: 8
                            text: modelData; color: ThemeModel.text; font.pixelSize: 13
                            font.family: "Consolas, 'Courier New', monospace"
                        }
                        MouseArea {
                            id: autoMouse
                            anchors.fill: parent; hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (activeTab) activeTab.insertText(modelData)
                                autoCompletePopup.close()
                            }
                        }
                    }
                }
            }

            // 执行历史面板
            Rectangle {
                id: historyPanel
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                radius: 12; color: ThemeModel.panel
                border.color: ThemeModel.border; visible: false

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 12; spacing: 4
                    RowLayout {
                        Text {
                            text: qsTr("Execution History"); color: ThemeModel.text
                            font.pixelSize: 13; font.bold: true
                        }
                        Item { Layout.fillWidth: true }
                        NeumorphismButton {
                            text: qsTr("Clear"); implicitWidth: 60; implicitHeight: 24
                            onClicked: {
                                if (typeof AppBridge !== 'undefined')
                                    AppBridge.sqlHistoryModel.clear()
                            }
                        }
                    }
                    ListView {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        model: typeof AppBridge !== 'undefined'
                               ? AppBridge.sqlHistoryModel : null
                        clip: true
                        delegate: Rectangle {
                            width: ListView.view.width; height: 32
                            color: hMouse.containsMouse ? ThemeModel.hover : "transparent"
                            radius: 4
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 8; spacing: 12
                                Text {
                                    text: sql || ""; color: ThemeModel.text
                                    font.pixelSize: 12; elide: Text.ElideRight
                                    font.family: "Consolas, 'Courier New', monospace"
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: elapsedMs.toFixed(1) + "ms"
                                    color: ThemeModel.textMuted; font.pixelSize: 11
                                }
                            }
                            MouseArea {
                                id: hMouse; anchors.fill: parent; hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: { if (activeTab) activeTab.setText(sql || "") }
                            }
                        }
                    }
                }
            }
        }
    }

    // ===== 编辑器组件 =====
    Component {
        id: editorComponent

        ScrollView {
            property alias text: editor.text
            property alias selectedText: editor.selectedText
            property alias cursorLine: editor.cursorRectangle.y
            property alias cursorColumn: editor.cursorRectangle.x
            property alias cursorY: editor.cursorRectangle.y

            function setText(t) { editor.text = t }
            function insertSql(sql) { editor.insert(editor.cursorPosition, sql) }
            function insertText(t) { editor.insert(editor.cursorPosition, t) }
            function clear() { editor.clear() }

            clip: true

            TextArea {
                id: editor
                font.family: "Consolas, 'Courier New', monospace"
                font.pixelSize: 14
                color: ThemeModel.text
                textFormat: TextEdit.PlainText
                wrapMode: TextEdit.NoWrap
                selectByMouse: true
                tabStopDistance: 32
                persistentSelection: true

                placeholderText: qsTr("-- SQL Editor\n-- Ctrl+Enter: Execute\n-- Ctrl+Space: Auto-complete\n\nSELECT * FROM table_name LIMIT 10;")
                placeholderTextColor: ThemeModel.textMuted

                background: Rectangle { color: "transparent" }

                // SQL 关键字自动补全列表
                property var keywords: [
                    "SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES",
                    "UPDATE", "SET", "DELETE", "CREATE", "TABLE", "ALTER",
                    "DROP", "INDEX", "VIEW", "TRIGGER", "PRIMARY", "KEY",
                    "FOREIGN", "REFERENCES", "NOT", "NULL", "DEFAULT",
                    "CHECK", "UNIQUE", "CONSTRAINT", "JOIN", "LEFT", "RIGHT",
                    "INNER", "OUTER", "CROSS", "ON", "AS", "AND", "OR",
                    "IN", "BETWEEN", "LIKE", "IS", "ORDER", "BY", "ASC",
                    "DESC", "GROUP", "HAVING", "LIMIT", "OFFSET", "UNION",
                    "ALL", "DISTINCT", "CASE", "WHEN", "THEN", "ELSE", "END",
                    "BEGIN", "COMMIT", "ROLLBACK", "TRANSACTION", "PRAGMA",
                    "EXPLAIN", "EXISTS", "IF", "REPLACE", "WITH", "RECURSIVE",
                    "INTEGER", "TEXT", "REAL", "BLOB", "NUMERIC", "BOOLEAN",
                    "AUTOINCREMENT", "ROWID", "COUNT", "SUM", "AVG", "MAX", "MIN",
                    "COALESCE", "IFNULL", "NULLIF", "GROUP_CONCAT", "SUBSTR",
                    "LENGTH", "UPPER", "LOWER", "TRIM", "REPLACE", "ABS", "ROUND"
                ]

                Keys.onPressed: (event) => {
                    // Ctrl+Enter: 执行
                    if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                        && (event.modifiers & Qt.ControlModifier)) {
                        executeCurrentSql()
                        event.accepted = true
                    }
                    // Ctrl+Space: 自动补全
                    else if (event.key === Qt.Key_Space
                             && (event.modifiers & Qt.ControlModifier)) {
                        triggerAutoComplete()
                        event.accepted = true
                    }
                    // Ctrl+/: 注释切换
                    else if (event.key === Qt.Key_Slash
                             && (event.modifiers & Qt.ControlModifier)) {
                        toggleComment()
                        event.accepted = true
                    }
                    // Tab: 缩进
                    else if (event.key === Qt.Key_Tab) {
                        editor.insert(editor.cursorPosition, "    ")
                        event.accepted = true
                    }
                }
            }
        }
    }

    // 自动补全项
    property var autoCompleteItems: []

    // 初始化第一个标签页
    Component.onCompleted: addTab()

    // ===== 操作方法 =====

    function addTab() {
        var tab = editorComponent.createObject(editorStack, {})
        var tabNum = editorStack.depth + 1
        tabs.push(tab)
        editorStack.push(tab)
        activeTab = tab

        // 创建标签页按钮
        var btn = Qt.createQmlObject(
            'import QtQuick; import QtQuick.Controls; Rectangle { ' +
            'property alias text: t.text; ' +
            'width: 120; height: 28; radius: 6; ' +
            'color: ThemeModel.panel; border.color: ThemeModel.border; ' +
            'Text { id: t; anchors.centerIn: parent; text: "' +
            qsTr("Query %1").arg(tabNum) +
            '"; color: ThemeModel.text; font.pixelSize: 12 } ' +
            'MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; ' +
            'onClicked: { editorPage.activeTab = tab; editorStack.currentIndex = ' +
            (editorStack.depth - 1) + ' } } }',
            tabRow, "tabBtn" + tabNum)
    }

    function executeCurrentSql() {
        if (typeof AppBridge === 'undefined' || !activeTab) return
        var sql = activeTab.selectedText || activeTab.text
        if (sql.trim().length === 0) {
            _toast.show(qsTr("No SQL to execute"), "error")
            return
        }
        var statements = sql.split(";").filter(function(s) { return s.trim().length > 0 })
        for (var i = 0; i < statements.length; i++)
            AppBridge.executeSql(statements[i].trim() + ";")
        _toast.show(qsTr("Executed %1 statement(s)").arg(statements.length), "success")
    }

    function formatCurrentSql() {
        if (!activeTab) return
        // 基础格式化：关键字大写 + 换行
        var kws = activeTab.keywords
        var text = activeTab.text
        for (var i = 0; i < kws.length; i++) {
            text = text.replace(new RegExp("\\b" + kws[i] + "\\b", "gi"), kws[i])
        }
        activeTab.text = text
        _toast.show(qsTr("SQL formatted"), "success")
    }

    function explainCurrentSql() {
        if (!activeTab) return
        var sql = "EXPLAIN QUERY PLAN " + activeTab.text
        if (typeof AppBridge !== 'undefined')
            AppBridge.executeSql(sql)
    }

    function toggleComment() {
        if (!activeTab) return
        var cursor = activeTab.cursorPosition
        var text = activeTab.text
        var pos = text.lastIndexOf("\n", cursor - 1)
        pos = pos < 0 ? 0 : pos + 1
        var lineStart = pos
        var lineEnd = text.indexOf("\n", cursor)
        lineEnd = lineEnd < 0 ? text.length : lineEnd
        var line = text.substring(lineStart, lineEnd)

        if (line.trimStart().startsWith("--")) {
            line = line.replace("--", "")
            activeTab.text = text.substring(0, lineStart) + line + text.substring(lineEnd)
        } else {
            activeTab.text = text.substring(0, lineStart) + "-- " + line + text.substring(lineEnd)
        }
    }

    function triggerAutoComplete() {
        if (!activeTab) return
        var cursor = activeTab.cursorPosition
        var text = activeTab.text
        // 获取光标前的 token
        var start = cursor - 1
        while (start >= 0 && /[a-zA-Z_]/.test(text[start])) start--
        var partial = text.substring(start + 1, cursor).toUpperCase()

        autoCompleteItems = activeTab.keywords.filter(function(kw) {
            return kw.startsWith(partial)
        })
        if (autoCompleteItems.length > 0)
            autoCompletePopup.open()
    }

    // 删除标签页
    function removeTab(index) {
        if (editorStack.depth <= 1) return
        var tab = editorStack.get(index)
        tabs.splice(index, 1)
        tab.destroy()
        if (activeTab === tab)
            activeTab = tabs[Math.min(index, tabs.length - 1)]
    }
}
