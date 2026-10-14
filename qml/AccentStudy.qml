pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import "packs/accents" as Accents

Window {
    id: root
    width: 1080
    height: 480
    visible: true
    color: "#10171c"
    title: "OmaFrames · Fireflies, Frost & Railway"
    property bool automatedCapture: false
    property bool motionEnabled: true
    property real elapsed: 0

    FrameAnimation {
        running: root.motionEnabled && !root.automatedCapture
        onTriggered: root.elapsed += frameTime
    }
    Rectangle { anchors.fill: parent; color: "#10171c" }
    Column {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 22
        Text { text: "Three little worlds around your windows"; color: "#e6eef2"; font.pixelSize: 26; font.bold: true }
        Text { text: "OmaFrames / effect studies"; color: "#8299a5"; font.pixelSize: 13 }
        Row {
            spacing: 24
            Repeater {
                model: ["fireflies", "frost", "railway"]
                delegate: Column {
                    id: card
                    required property string modelData
                    width: 320
                    spacing: 12
                    Item {
                        width: 320; height: 240
                        Rectangle {
                            anchors.fill: parent; anchors.margins: 20
                            color: "#19252d"; radius: 8
                            border.color: "#3a4c58"
                            Column {
                                anchors.fill: parent; anchors.margins: 20; spacing: 14
                                Row {
                                    spacing: 6
                                    Repeater {
                                        model: ["#e17c75", "#e6c079", "#8abf9c"]
                                        delegate: Rectangle {
                                            required property string modelData
                                            width: 6; height: 6; radius: 3; color: modelData
                                        }
                                    }
                                }
                                Text { text: "~ / a quiet little window"; color: "#9aafbc"; font.family: "monospace"; font.pixelSize: 11 }
                                Repeater {
                                    model: [175, 210, 140, 190]
                                    delegate: Rectangle {
                                        required property int modelData
                                        width: modelData; height: 4; radius: 2; color: "#2c3e49"
                                    }
                                }
                            }
                        }
                        Accents.AccentFrame {
                            anchors.fill: parent
                            pack: card.modelData
                            elapsed: root.automatedCapture ? 7 : root.elapsed
                            motionEnabled: root.motionEnabled
                            growth: root.automatedCapture || !root.motionEnabled ? 1 : Math.min(1, root.elapsed/1.6)
                        }
                    }
                    Text { text: card.modelData.charAt(0).toUpperCase()+card.modelData.slice(1); color: "#e6eef2"; font.pixelSize: 18 }
                    Text {
                        text: card.modelData === "fireflies" ? "A soft glow, always wandering."
                            : card.modelData === "frost" ? "Ice branches quietly into place." : "The scenic route around your work."
                        color: "#8299a5"; font.pixelSize: 12
                    }
                }
            }
        }
        Button {
            visible: !root.automatedCapture
            text: root.motionEnabled ? "Pause motion" : "Resume motion"
            onClicked: root.motionEnabled = !root.motionEnabled
        }
    }
    Timer {
        running: root.automatedCapture
        interval: 700
        onTriggered: root.contentItem.grabToImage(function(result) {
            result.saveToFile(Qt.resolvedUrl("../captures/accent-study.png").toString().replace("file://", ""))
            Qt.quit()
        })
    }
}
