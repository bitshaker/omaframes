pragma ComponentBehavior: Bound

// OmaFrames Vines pack leaf primitive.
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
                path: "M 1 12 C 6 10, 5 5, 11 3 C 17 0, 24 5, 33 11 C 27 18, 19 23, 12 21 C 6 19, 7 14, 1 12 Z"
            }
        }

        ShapePath {
            fillColor: "transparent"
            strokeColor: Qt.rgba(leaf.veinColor.r, leaf.veinColor.g, leaf.veinColor.b, 0.78)
            strokeWidth: 1.15
            capStyle: ShapePath.RoundCap

            PathSvg {
                path: "M 3 12 C 12 12, 21 12, 30 11 M 12 12 L 8 6 M 19 12 L 16 5 M 12 12 L 8 18 M 19 12 L 16 20"
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
