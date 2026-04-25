import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property string title: "Param"
    property int from: 0
    property int to: 100
    property int value: 50
    signal changed(int value)
    height: 58
    Column {
        anchors.fill: parent
        spacing: 3
        Row {
            width: parent.width
            Label { text: root.title; color: "#a9c6d6"; font.pixelSize: 13; width: parent.width - 48 }
            Label { text: slider.value.toFixed(0); color: "#e7f6ff"; font.pixelSize: 13; horizontalAlignment: Text.AlignRight; width: 46 }
        }
        Slider {
            id: slider
            width: parent.width
            from: root.from
            to: root.to
            stepSize: 1
            value: root.value
            onMoved: root.changed(value)
        }
    }
}
