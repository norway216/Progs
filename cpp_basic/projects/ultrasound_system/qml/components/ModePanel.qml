import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

Rectangle {
    id: root
    property var imageManager
    color: "#0c1720"
    radius: 12
    border.color: "#223949"
    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10
        Label { text: "Scan Mode Updater Pool"; color: "#e8f7ff"; font.pixelSize: 16; font.bold: true }
        Grid {
            columns: 2
            spacing: 8
            Repeater {
                model: ["B", "M", "Color", "PW", "BC", "Triplex", "4D", "Elasto", "FreeM", "Panoramic"]
                ControlButton { width: 116; text: modelData; accent: root.imageManager && root.imageManager.scanMode === modelData ? "#00acc1" : "#263c4b"; onClicked: root.imageManager.switchScanMode(modelData) }
            }
        }
    }
}
