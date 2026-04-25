import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "components"

ApplicationWindow {
    id: root
    visible: true
    width: 1440
    height: 900
    title: "QML + C++17 Ultrasound Demo"
    color: "#071018"

    PatientDialog { id: patientDialog; exam: app.exam }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            radius: 14
            color: "#0d1b25"
            border.color: "#203848"
            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12
                Label { text: "SonoAir QML Demo"; color: "#f2fbff"; font.pixelSize: 25; font.bold: true; Layout.preferredWidth: 260 }
                StatusBadge { text: app.state.state; accent: app.image.frozen ? "#ffca28" : "#26c6da" }
                StatusBadge { text: app.peripheral.probeConnected ? app.peripheral.probeName : "No Probe"; accent: app.peripheral.probeConnected ? "#66bb6a" : "#ef5350" }
                Label { text: "Patient: " + app.exam.patientName + " / " + app.exam.patientId; color: "#bcd7e6"; font.pixelSize: 14; Layout.fillWidth: true }
                Label { text: "FPS " + app.image.fps.toFixed(1); color: "#90caf9"; font.pixelSize: 16; font.bold: true }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                Layout.preferredWidth: 300
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 256
                    radius: 12
                    color: "#0c1720"
                    border.color: "#223949"
                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10
                        Label { text: "Workflow Controls"; color: "#e8f7ff"; font.pixelSize: 16; font.bold: true }
                        ControlButton { width: parent.width; text: app.peripheral.probeConnected ? "Remove Probe" : "Insert Probe"; accent: "#00897b"; onClicked: app.peripheral.toggleProbe() }
                        Row { spacing: 8; ControlButton { width: 130; text: "Freeze"; accent: "#fb8c00"; onClicked: app.state.postEvent("freeze") } ControlButton { width: 130; text: "Unfreeze"; accent: "#43a047"; onClicked: app.state.postEvent("unfreeze") } }
                        Row { spacing: 8; ControlButton { width: 130; text: "Save"; accent: "#5e35b1"; onClicked: app.exam.saveCurrentImage() } ControlButton { width: 130; text: "PACS"; accent: "#3949ab"; onClicked: app.exam.sendToPacs() } }
                        Row { spacing: 8; ControlButton { width: 130; text: "Patient"; accent: "#546e7a"; onClicked: patientDialog.open() } ControlButton { width: 130; text: "USB"; accent: "#546e7a"; onClicked: app.peripheral.saveToUsb() } }
                    }
                }

                ModePanel { Layout.fillWidth: true; Layout.preferredHeight: 360; imageManager: app.image }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 12
                    color: "#0c1720"
                    border.color: "#223949"
                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10
                        Label { text: "Preset / Parameters"; color: "#e8f7ff"; font.pixelSize: 16; font.bold: true }
                        Row { spacing: 8; ControlButton { width: 82; text: "ABD"; accent: "#263c4b"; onClicked: app.preset.switchPreset("Abdomen") } ControlButton { width: 82; text: "CARD"; accent: "#263c4b"; onClicked: app.preset.switchPreset("Cardiac") } ControlButton { width: 82; text: "VAS"; accent: "#263c4b"; onClicked: app.preset.switchPreset("Vascular") } }
                        ParameterSlider { width: parent.width; title: "Depth"; from: 40; to: 200; value: app.params.depth; onChanged: app.params.depth = value }
                        ParameterSlider { width: parent.width; title: "Gain"; from: 0; to: 100; value: app.params.gain; onChanged: app.params.gain = value }
                        ParameterSlider { width: parent.width; title: "Dynamic Range"; from: 30; to: 120; value: app.params.dynamicRange; onChanged: app.params.dynamicRange = value }
                        ParameterSlider { width: parent.width; title: "Frequency MHz"; from: 2; to: 12; value: app.params.frequency; onChanged: app.params.frequency = value }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12
                ImageViewport { Layout.fillWidth: true; Layout.fillHeight: true; frameNo: app.image.frameNo; stateText: app.state.state; modeText: app.image.scanMode }
                CineBar { Layout.fillWidth: true; stateManager: app.state; cine: app.cine }
            }

            ColumnLayout {
                Layout.preferredWidth: 330
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 230
                    radius: 12
                    color: "#0c1720"
                    border.color: "#223949"
                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10
                        Label { text: "Mark / Measurement"; color: "#e8f7ff"; font.pixelSize: 16; font.bold: true }
                        Row { spacing: 8; ControlButton { width: 96; text: "Distance"; accent: "#00695c"; onClicked: { app.mark.activateTool("Distance"); app.state.postEvent("measureStart") } } ControlButton { width: 96; text: "BodyMark"; accent: "#00695c"; onClicked: app.mark.addComment("BodyMark: Abdomen") } ControlButton { width: 96; text: "Clear"; accent: "#455a64"; onClicked: app.mark.clear() } }
                        ControlButton { width: parent.width; text: "Simulate Distance Measurement"; accent: "#00838f"; onClicked: { app.mark.addDistanceMeasurement(120, 210, 430, 295); app.state.postEvent("measureComplete") } }
                        ListView {
                            width: parent.width; height: 105; clip: true; model: app.mark.measurements
                            delegate: Label { width: parent.width; text: modelData.type + ": " + modelData.value; color: "#d3edf7"; font.pixelSize: 13; padding: 4 }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 12
                    color: "#0c1720"
                    border.color: "#223949"
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8
                        RowLayout { Layout.fillWidth: true; Label { text: "Runtime Business Logs"; color: "#e8f7ff"; font.pixelSize: 16; font.bold: true; Layout.fillWidth: true } Button { text: "Clear"; onClicked: app.clearLogs() } }
                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: app.logs
                            delegate: Text { width: ListView.view.width; text: modelData; color: "#a7c7d8"; font.pixelSize: 12; wrapMode: Text.WrapAnywhere; padding: 4 }
                        }
                    }
                }
            }
        }
    }
}
