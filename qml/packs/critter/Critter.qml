pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: root

    width: 72
    height: 56

    property string pose: "idle"
    property string edge: "top"
    property bool facingPositive: true
    property color bodyColor: "#83c96f"
    property color accentColor: "#d9ef9b"

    readonly property real edgeAngle: edge === "right" ? 90
        : edge === "bottom" ? 180
        : edge === "left" ? 270
        : 0
    readonly property bool airborne: pose === "flight"
    readonly property bool crouched: pose === "crouch" || pose === "land"
    readonly property real gait: pose === "walkB" ? -1 : pose === "walkA" ? 1 : 0

    Item {
        id: creature

        anchors.centerIn: parent
        width: 66
        height: 46
        y: root.airborne ? -3 : root.crouched ? 3 : 0
        rotation: root.edgeAngle + (root.airborne ? -12 : 0)

        transform: Scale {
            origin.x: creature.width / 2
            origin.y: creature.height / 2
            xScale: root.facingPositive ? 1 : -1
        }

        Shape {
            anchors.fill: parent
            antialiasing: true

            ShapePath {
                fillColor: "transparent"
                strokeColor: root.bodyColor
                strokeWidth: 8
                capStyle: ShapePath.RoundCap

                PathMove { x: 5; y: 27 }
                PathCubic {
                    x: 31
                    y: 24
                    control1X: root.airborne ? 13 : 9
                    control1Y: root.airborne ? 5 : 42
                    control2X: root.airborne ? 22 : 23
                    control2Y: root.airborne ? 37 : 19
                }
            }
        }

        Rectangle {
            x: 25
            y: root.crouched ? 17 : 14
            width: 29
            height: root.crouched ? 18 : 22
            radius: height / 2
            color: root.bodyColor
            rotation: root.airborne ? -7 : 0
        }

        Rectangle {
            x: 49
            y: root.crouched ? 15 : 12
            width: 16
            height: root.crouched ? 17 : 20
            radius: 8
            color: root.bodyColor
            rotation: root.airborne ? -13 : 0

            Rectangle {
                x: 9
                y: 4
                width: 3.5
                height: 3.5
                radius: width / 2
                color: "#102018"
            }

            Rectangle {
                x: 11.5
                y: 5
                width: 1
                height: 1
                radius: width / 2
                color: root.accentColor
            }
        }

        Repeater {
            model: [
                { "x": 29, "front": false, "upper": true },
                { "x": 34, "front": false, "upper": false },
                { "x": 46, "front": true, "upper": true },
                { "x": 49, "front": true, "upper": false }
            ]

            Item {
                id: leg

                required property var modelData
                readonly property bool upper: modelData.upper
                readonly property bool front: modelData.front

                x: modelData.x
                y: upper ? 15 : 28
                width: 16
                height: 8
                rotation: root.airborne
                    ? (front ? -42 : 38) * (upper ? 1 : -1)
                    : root.crouched
                        ? (front ? 24 : -18) * (upper ? 1 : -1)
                        : (front ? -10 : 8) + root.gait * (upper ? 18 : -18)

                Rectangle {
                    width: 12
                    height: 4
                    radius: 2
                    color: root.bodyColor
                }

                Rectangle {
                    x: 9
                    y: leg.upper ? -2 : 2
                    width: 7
                    height: 3
                    radius: 1.5
                    color: root.accentColor
                    rotation: leg.upper ? -18 : 18
                }
            }
        }

        Rectangle {
            x: 37
            y: root.crouched ? 21 : 19
            width: 11
            height: 4
            radius: 2
            color: root.accentColor
            opacity: 0.62
        }
    }
}
