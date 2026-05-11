import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    width: 800
    height: 600
    color: "#222"

    Image {
        id: videoDisplay
        anchors.fill: parent
        source: ""
    }

    Rectangle {
        width: 200
        height: 100
        anchors.top: parent.top
        anchors.right: parent.right
        color: "#333"
        radius: 10
        border.color: "white"
        border.width: 1
        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 5
            Text { text: "姓名: " + faceRec.name; color: "white"; font.bold: true }
            Text { text: "编号: " + faceRec.id; color: "white"; font.bold: true }
            Text { text: "识别结果: " + faceRec.result; color: "white"; font.bold: true }
        }
    }

    Connections {
        target: faceRec
        onFrameReady: {
            videoDisplay.source = img
        }
    }
}