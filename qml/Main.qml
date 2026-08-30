import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 1200
    height: 780
    minimumWidth: 900
    minimumHeight: 620
    visible: true
    color: "#07100c"
    title: "OmaVines — QML visual study"

    Study {
        anchors.fill: parent
    }
}
