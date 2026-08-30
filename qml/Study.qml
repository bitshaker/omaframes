pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import "packs/critter" as Critter
import "packs/vines" as Vines

Rectangle {
    id: root

    width: 1200
    height: 780
    color: "#07100c"

    property real growthProgress: 0
    property int growthDuration: 6200
    property bool motionEnabled: true
    property real critterProgress: automatedCapture ? 0.53 : 0
    property string capturePath: ""
    property bool automatedCapture: false

    readonly property real jumpProgress: Math.max(0, Math.min(1, (critterProgress - 0.30) / 0.42))
    readonly property bool critterFlying: critterProgress >= 0.30 && critterProgress < 0.72
    readonly property point critterStart: Qt.point(sourceWindow.x + sourceWindow.width + 13,
                                                   sourceWindow.y + 122)
    readonly property point critterFinish: Qt.point(targetWindow.x + 105,
                                                    targetWindow.y + targetWindow.height + 13)
    readonly property point critterApex: Qt.point((critterStart.x + critterFinish.x) / 2,
                                                  Math.min(critterStart.y, critterFinish.y) - 130)

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
            text: "OMAFRAMES / VINES + CHAMELEON / STUDY 02"
            color: "#8ccf82"
            font.pixelSize: 13
            font.letterSpacing: 2.4
            font.weight: Font.DemiBold
        }

        Text {
            text: "A tiny living world on your window edges"
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
            text: "PROTOTYPE • NATIVE + VECTOR QML"
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

            x: sourceWindow.x + 12
            y: sourceWindow.y + 16
            width: sourceWindow.width
            height: sourceWindow.height
            radius: 20
            color: "#99000000"
        }

        Rectangle {
            id: sourceWindow

            x: 36
            y: 158
            width: Math.min(650, stage.width - 420)
            height: 348
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
                        width: Math.min(460, sourceWindow.width - 150)
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

            x: sourceWindow.x - 48
            y: sourceWindow.y - 48
            width: sourceWindow.width + 96
            height: sourceWindow.height + 96
            progress: root.growthProgress
            motionEnabled: root.motionEnabled && !root.automatedCapture
        }

        Rectangle {
            x: targetWindow.x + 10
            y: targetWindow.y + 13
            width: targetWindow.width
            height: targetWindow.height
            radius: 17
            color: "#99000000"
        }

        Rectangle {
            id: targetWindow

            x: stage.width - width - 32
            y: 34
            width: 310
            height: 214
            radius: 15
            color: "#151c19"
            border.color: "#39503e"
            border.width: 1
            clip: true

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 42
                color: "#1b2720"

                Text {
                    anchors.centerIn: parent
                    text: "seedlings.todo"
                    color: "#839287"
                    font.family: "monospace"
                    font.pixelSize: 11
                }
            }

            Column {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 24
                anchors.topMargin: 68
                spacing: 13

                Repeater {
                    model: ["find a sunny edge", "stretch", "make the leap"]

                    Row {
                        required property string modelData
                        spacing: 10

                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: "#78ad70"
                        }

                        Text {
                            text: parent.modelData
                            color: "#9aaca0"
                            font.family: "monospace"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        Shape {
            visible: root.critterFlying
            anchors.fill: parent
            opacity: 0.22

            ShapePath {
                fillColor: "transparent"
                strokeColor: "#9ed38c"
                strokeWidth: 1
                strokeStyle: ShapePath.DashLine
                dashPattern: [4, 5]

                PathMove { x: root.critterStart.x; y: root.critterStart.y }
                PathQuad {
                    x: root.critterFinish.x
                    y: root.critterFinish.y
                    controlX: root.critterApex.x
                    controlY: root.critterApex.y
                }
            }
        }

        Critter.Critter {
            id: critter

            readonly property real flightX: Math.pow(1 - root.jumpProgress, 2) * root.critterStart.x
                + 2 * (1 - root.jumpProgress) * root.jumpProgress * root.critterApex.x
                + Math.pow(root.jumpProgress, 2) * root.critterFinish.x
            readonly property real flightY: Math.pow(1 - root.jumpProgress, 2) * root.critterStart.y
                + 2 * (1 - root.jumpProgress) * root.jumpProgress * root.critterApex.y
                + Math.pow(root.jumpProgress, 2) * root.critterFinish.y

            x: (root.critterFlying
                ? flightX
                : root.critterProgress < 0.30
                    ? root.critterStart.x
                    : root.critterFinish.x + Math.min(1, (root.critterProgress - 0.72) / 0.28) * 95) - width / 2
            y: (root.critterFlying
                ? flightY
                : root.critterProgress < 0.30
                    ? root.critterStart.y - 42 * (root.critterProgress / 0.30)
                    : root.critterFinish.y) - height / 2
            edge: root.critterProgress < 0.30 ? "right" : root.critterFlying ? "top" : "bottom"
            pose: root.critterFlying ? "flight"
                : root.critterProgress > 0.68 && root.critterProgress < 0.79 ? "land"
                : root.critterProgress > 0.25 && root.critterProgress < 0.30 ? "crouch"
                : Math.floor(root.critterProgress * 16) % 2 ? "walkA" : "walkB"
            facingPositive: true
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

    SequentialAnimation on critterProgress {
        running: root.motionEnabled && !root.automatedCapture
        loops: Animation.Infinite

        NumberAnimation { from: 0; to: 0.25; duration: 2100; easing.type: Easing.InOutSine }
        NumberAnimation { from: 0.25; to: 0.30; duration: 420; easing.type: Easing.InCubic }
        NumberAnimation { from: 0.30; to: 0.72; duration: 920; easing.type: Easing.InOutQuad }
        NumberAnimation { from: 0.72; to: 0.79; duration: 360; easing.type: Easing.OutBack }
        NumberAnimation { from: 0.79; to: 1; duration: 1900; easing.type: Easing.InOutSine }
        PauseAnimation { duration: 900 }
    }

}
