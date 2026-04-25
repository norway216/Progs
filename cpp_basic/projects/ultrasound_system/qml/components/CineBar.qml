import QtQuick 2.15
import QtQuick.Controls 2.15
import "."

Rectangle {
    id: root
    property var stateManager
    property var cine
    color: "#0c1720"
    radius: 12
    border.color: "#223949"
    height: 76
    Row {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12
        ControlButton { text: "Play Cine"; width: 110; accent: "#546e7a"; onClicked: root.stateManager.postEvent("cinePlay") }
        ControlButton { text: "Stop"; width: 90; accent: "#546e7a"; onClicked: root.stateManager.postEvent("cineStop") }
        Slider { id: s; width: 420; from: 0; to: Math.max(0, root.cine.size - 1); stepSize: 1; value: root.cine.currentIndex; onMoved: root.cine.currentIndex = value }
        Label { text: root.cine.currentIndex + " / " + Math.max(0, root.cine.size - 1); color: "#d9edf7"; anchors.verticalCenter: parent.verticalCenter }
    }
}
