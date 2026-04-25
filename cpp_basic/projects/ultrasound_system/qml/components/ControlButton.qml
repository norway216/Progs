import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: root
    property color accent: "#1e88e5"
    height: 44
    font.pixelSize: 14
    font.bold: true
    background: Rectangle {
        radius: 10
        color: root.down ? Qt.darker(root.accent, 1.25) : root.accent
        border.color: Qt.lighter(root.accent, 1.25)
        border.width: 1
    }
    contentItem: Text { text: root.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font: root.font }
}
