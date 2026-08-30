pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "packs/vines" as Vines

Rectangle {
    id: root

    width: 1200
    height: 780
    color: "#07100c"

    property real growthProgress: 0
    property int growthDuration: 6200
    property bool motionEnabled: true
    property string capturePath: ""
    property bool automatedCapture: false

    function regrow() {
        growthAnimation.stop()
        growthProgress = 0
        growthAnimation.restart()
    }

    Rectangle {
        anchors.fill: parent

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#07100c" }
            GradientStop { position: 0.52; color: "#0b1912" }
            GradientStop { position: 1.0; color: "#10160f" }
        }
    }

    Repeater {
        model: 8

        Rectangle {
            required property int index
            readonly property real diameter: 130 + index * 38

            x: (index * 247) % Math.max(1, root.width) - diameter / 2
            y: (index * 151) % Math.max(1, root.height) - diameter / 2
            width: diameter
            height: diameter
            radius: diameter / 2
            color: "transparent"
            border.width: 1
            border.color: index % 2 ? "#152a1c" : "#1a3022"
            opacity: 0.48
        }
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 42
        anchors.topMargin: 34
        spacing: 5

        Text {
            text: "OMAFRAMES / VINES / STUDY 01"
            color: "#8ccf82"
            font.pixelSize: 13
            font.letterSpacing: 2.4
            font.weight: Font.DemiBold
        }

        Text {
            text: "A living edge for Hyprland"
            color: "#edf3e7"
            font.pixelSize: 25
            font.weight: Font.Medium
        }
    }

    Rectangle {
        id: experimentBadge

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 42
        anchors.topMargin: 38
        width: badgeLabel.implicitWidth + 28
        height: 30
        radius: 15
        color: "#15281b"
        border.color: "#2e5537"

        Text {
            id: badgeLabel
            anchors.centerIn: parent
            text: "PROTOTYPE • VECTOR QML"
            color: "#9fc79c"
            font.pixelSize: 11
            font.letterSpacing: 1.2
            font.weight: Font.DemiBold
        }
    }

    Item {
        id: stage

        anchors.fill: parent
        anchors.topMargin: 102
        anchors.bottomMargin: 96
        anchors.leftMargin: 58
        anchors.rightMargin: 58

        Rectangle {
            id: windowShadow

            anchors.centerIn: parent
            width: Math.min(850, stage.width - 80)
            height: Math.min(480, stage.height - 72)
            x: 12
            y: 19
            radius: 20
            color: "#99000000"
        }

        Rectangle {
            id: sampleWindow

            anchors.centerIn: parent
            width: windowShadow.width
            height: windowShadow.height
            radius: 17
            color: "#121b17"
            border.color: "#2a3a30"
            border.width: 1
            clip: true

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 48
                color: "#17231c"

                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 18
                    spacing: 8

                    Repeater {
                        model: ["#d16b62", "#d2aa64", "#6eae75"]

                        Rectangle {
                            required property string modelData
                            width: 10
                            height: 10
                            radius: 5
                            color: modelData
                            opacity: 0.88
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: "forest-notes.md"
                    color: "#718177"
                    font.family: "monospace"
                    font.pixelSize: 12
                }
            }

            Item {
                anchors.fill: parent
                anchors.topMargin: 48

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 58
                    color: "#101713"
                }

                Column {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 88
                    anchors.topMargin: 50
                    spacing: 17

                    Text {
                        text: "digital garden"
                        color: "#d5dfd2"
                        font.family: "monospace"
                        font.pixelSize: 23
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: "01  grow where the compositor gives you light"
                        color: "#71877a"
                        font.family: "monospace"
                        font.pixelSize: 14
                    }

                    Text {
                        text: "02  follow the window, never steal its input"
                        color: "#71877a"
                        font.family: "monospace"
                        font.pixelSize: 14
                    }

                    Text {
                        text: "03  let quiet motion make the desktop feel alive"
                        color: "#71877a"
                        font.family: "monospace"
                        font.pixelSize: 14
                    }

                    Rectangle {
                        width: Math.min(560, sampleWindow.width - 150)
                        height: 1
                        color: "#26352b"
                    }

                    Text {
                        text: "The Vines pack owns its shapes, color and motion.\nOmaFrames supplies the window lifecycle and compositor bridge."
                        color: "#9caf9f"
                        font.family: "monospace"
                        font.pixelSize: 13
                        lineHeight: 1.55
                    }
                }
            }
        }

        Vines.VineFrame {
            id: vines

            x: sampleWindow.x - 48
            y: sampleWindow.y - 48
            width: sampleWindow.width + 96
            height: sampleWindow.height + 96
            progress: root.growthProgress
            motionEnabled: root.motionEnabled && !root.automatedCapture
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 78
        color: "#0b120e"
        border.color: "#1a281f"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 42
            anchors.rightMargin: 42
            spacing: 18

            Button {
                id: regrowButton
                text: "Regrow"
                onClicked: root.regrow()

                contentItem: Text {
                    text: regrowButton.text
                    color: "#e7efe2"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }

                background: Rectangle {
                    implicitWidth: 98
                    implicitHeight: 38
                    radius: 9
                    color: regrowButton.down ? "#315f3a" : "#244d30"
                    border.color: "#4b8055"
                }
            }

            Text {
                text: "GROWTH"
                color: "#68796d"
                font.pixelSize: 10
                font.letterSpacing: 1.5
                font.weight: Font.DemiBold
            }

            Slider {
                id: growthSlider
                Layout.preferredWidth: 210
                from: 0
                to: 1
                value: root.growthProgress
                onMoved: {
                    growthAnimation.stop()
                    root.growthProgress = value
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "organic sway"
                color: "#91a197"
                font.pixelSize: 12
            }

            Switch {
                checked: root.motionEnabled
                onToggled: root.motionEnabled = checked
            }

            Text {
                text: Math.round(root.growthProgress * 100) + "%"
                color: "#7ec083"
                font.family: "monospace"
                font.pixelSize: 12
                Layout.preferredWidth: 42
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    NumberAnimation {
        id: growthAnimation
        target: root
        property: "growthProgress"
        from: 0
        to: 1
        duration: root.growthDuration
        easing.type: Easing.InOutCubic
        running: !root.automatedCapture
    }

}
