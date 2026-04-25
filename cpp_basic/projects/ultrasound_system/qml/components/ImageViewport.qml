import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property int frameNo: 0
    property string stateText: "Idle"
    property string modeText: "B"
    color: "#02060a"
    radius: 12
    border.color: "#193142"
    border.width: 1
    clip: true
    Image {
        id: image
        anchors.fill: parent
        anchors.margins: 10
        fillMode: Image.PreserveAspectFit
        cache: false
        source: "image://ultrasound/live?frame=" + root.frameNo + "&t=" + Date.now()
    }
    Rectangle {
        anchors.left: parent.left; anchors.top: parent.top; anchors.margins: 18
        width: 230; height: 78; radius: 10
        color: "#66000000"
        border.color: "#33566d"
        Column {
            anchors.fill: parent; anchors.margins: 10; spacing: 4
            Label { text: "Mode: " + root.modeText; color: "#e4f6ff"; font.pixelSize: 15; font.bold: true }
            Label { text: "State: " + root.stateText; color: "#ffd54f"; font.pixelSize: 14 }
            Label { text: "Frame: " + root.frameNo; color: "#a7c7d8"; font.pixelSize: 13 }
        }
    }
}
