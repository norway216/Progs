import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    id: root
    property var exam
    modal: true
    title: "Patient Registration"
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 360
    Column {
        anchors.fill: parent
        spacing: 10
        TextField { id: nameField; width: parent.width; placeholderText: "Patient name"; text: root.exam ? root.exam.patientName : "" }
        TextField { id: idField; width: parent.width; placeholderText: "Patient ID"; text: root.exam ? root.exam.patientId : "" }
    }
    onAccepted: root.exam.registerPatient(nameField.text, idField.text)
}
