import QtQuick
import QtQuick.Controls

// 确认弹窗——拟态化卡片风格
Dialog {
    id: dialog

    property alias message: msgText.text
    property alias okText: okBtn.text
    property alias cancelText: cancelBtn.text

    signal accepted()
    signal rejected()

    modal: true
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        radius: ThemeModel.cardRadius
        color: ThemeModel.panel
        border.color: ThemeModel.border
    }

    contentItem: Column {
        spacing: 20
        width: 320

        Text {
            id: msgText
            width: parent.width
            wrapMode: Text.Wrap
            color: ThemeModel.text
            font.pixelSize: 14
        }

        Row {
            spacing: 12
            anchors.right: parent.right

            NeumorphismButton {
                id: cancelBtn
                text: qsTr("Cancel")
                onClicked: {
                    dialog.rejected()
                    dialog.close()
                }
            }

            NeumorphismButton {
                id: okBtn
                text: qsTr("OK")
                accent: true
                onClicked: {
                    dialog.accepted()
                    dialog.close()
                }
            }
        }
    }
}
