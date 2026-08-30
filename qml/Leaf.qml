pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: leaf

    property color leafColor: "#59ba65"
    property color veinColor: "#b7dc78"
    property bool swayEnabled: true
    property real swayAmount: 2.4
    property real restingAngle: 0

    width: 34
    height: 24
    rotation: restingAngle
    transformOrigin: Item.Left

    Shape {
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            fillColor: leaf.leafColor
            strokeColor: Qt.darker(leaf.leafColor, 1.45)
            strokeWidth: 1.1
            joinStyle: ShapePath.RoundJoin

            PathSvg {
                path: "M 1 12 C 7 1, 23 -2, 33 9 C 27 22, 10 27, 1 12 Z"
            }
        }

        ShapePath {
            fillColor: "transparent"
            strokeColor: Qt.rgba(leaf.veinColor.r, leaf.veinColor.g, leaf.veinColor.b, 0.78)
            strokeWidth: 1.15
            capStyle: ShapePath.RoundCap

            PathSvg {
                path: "M 3 12 C 12 11, 21 10, 30 8"
            }
        }
    }

    SequentialAnimation on rotation {
        running: leaf.swayEnabled && leaf.visible
        loops: Animation.Infinite

        NumberAnimation {
            from: leaf.restingAngle - leaf.swayAmount
            to: leaf.restingAngle + leaf.swayAmount
            duration: 1800
            easing.type: Easing.InOutSine
        }

        NumberAnimation {
            from: leaf.restingAngle + leaf.swayAmount
            to: leaf.restingAngle - leaf.swayAmount
            duration: 2200
            easing.type: Easing.InOutSine
        }
    }
}
