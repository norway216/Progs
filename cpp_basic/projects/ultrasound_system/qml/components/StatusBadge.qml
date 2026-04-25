import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string text: "READY"
    property color accent: "#29b6f6"
    radius: 8
    height: 28
    width: label.implicitWidth + 22
    color: Qt.rgba(accent.r, accent.g, accent.b, 0.18)
    border.color: accent
    border.width: 1
    Label { id: label; anchors.centerIn: parent; text: root.text; color: "#dff7ff"; font.pixelSize: 13; font.bold: true }
}
