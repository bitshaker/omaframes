pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: frame

    property real progress: 0
    property bool motionEnabled: true
    property color stemColor: "#3f9658"
    property color stemHighlight: "#8bcf72"
    property color leafColor: "#58b967"

    readonly property real inset: 34
    readonly property string perimeterPath: {
        const x0 = inset
        const x1 = width - inset
        const y0 = inset
        const y1 = height - inset

        return "M " + (x0 + 7) + " " + (y1 - 25)
             + " C " + (x0 - 13) + " " + (y1 - 93)
             + ", " + (x0 + 11) + " " + (y0 + 92)
             + ", " + (x0 + 2) + " " + (y0 + 31)
             + " C " + (x0 - 5) + " " + (y0 + 6)
             + ", " + (x0 + 8) + " " + (y0 - 5)
             + ", " + (x0 + 39) + " " + y0
             + " C " + (x0 + 139) + " " + (y0 + 15)
             + ", " + (x1 - 136) + " " + (y0 - 11)
             + ", " + (x1 - 35) + " " + (y0 + 1)
             + " C " + (x1 - 6) + " " + (y0 + 5)
             + ", " + (x1 + 8) + " " + (y0 + 17)
             + ", " + (x1 + 1) + " " + (y0 + 48)
             + " C " + (x1 - 12) + " " + (y0 + 121)
             + ", " + (x1 + 14) + " " + (y1 - 123)
             + ", " + (x1 - 2) + " " + (y1 - 38)
             + " C " + (x1 - 7) + " " + (y1 - 7)
             + ", " + (x1 - 25) + " " + (y1 + 8)
             + ", " + (x1 - 56) + " " + y1
             + " C " + (x1 - 153) + " " + (y1 - 17)
             + ", " + (x0 + 156) + " " + (y1 + 16)
             + ", " + (x0 + 49) + " " + (y1 - 1)
             + " C " + (x0 + 24) + " " + (y1 - 5)
             + ", " + (x0 + 13) + " " + (y1 - 11)
             + ", " + (x0 + 7) + " " + (y1 - 25)
    }

    Shape {
        anchors.fill: parent
        antialiasing: true
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            fillColor: "transparent"
            strokeColor: "#102f24"
            strokeWidth: 8
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            trim.start: 0
            trim.end: frame.progress

            PathSvg { path: frame.perimeterPath }
        }

        ShapePath {
            fillColor: "transparent"
            strokeColor: frame.stemColor
            strokeWidth: 4.2
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            trim.start: 0
            trim.end: frame.progress

            PathSvg { path: frame.perimeterPath }
        }

        ShapePath {
            fillColor: "transparent"
            strokeColor: Qt.rgba(frame.stemHighlight.r, frame.stemHighlight.g, frame.stemHighlight.b, 0.82)
            strokeWidth: 1.15
            capStyle: ShapePath.RoundCap
            trim.start: 0
            trim.end: frame.progress

            PathSvg { path: frame.perimeterPath }
        }
    }

    Repeater {
        model: ListModel {
            ListElement { nx: 0.038; ny: 0.76; angle: 168; reveal: 0.05; size: 0.82; tint: "#4ba85c" }
            ListElement { nx: 0.041; ny: 0.57; angle: 194; reveal: 0.12; size: 1.05; tint: "#63c66c" }
            ListElement { nx: 0.039; ny: 0.37; angle: 165; reveal: 0.20; size: 0.88; tint: "#3f9e54" }
            ListElement { nx: 0.073; ny: 0.064; angle: 244; reveal: 0.29; size: 1.08; tint: "#65bd61" }
            ListElement { nx: 0.245; ny: 0.052; angle: 266; reveal: 0.38; size: 0.76; tint: "#4da85d" }
            ListElement { nx: 0.46; ny: 0.050; angle: 80; reveal: 0.46; size: 0.96; tint: "#6bc96d" }
            ListElement { nx: 0.686; ny: 0.052; angle: 258; reveal: 0.53; size: 0.83; tint: "#4aa659" }
            ListElement { nx: 0.904; ny: 0.066; angle: 292; reveal: 0.61; size: 1.04; tint: "#68c66b" }
            ListElement { nx: 0.949; ny: 0.28; angle: -13; reveal: 0.69; size: 0.82; tint: "#4aa65c" }
            ListElement { nx: 0.955; ny: 0.54; angle: 14; reveal: 0.76; size: 1.05; tint: "#64bd64" }
            ListElement { nx: 0.946; ny: 0.77; angle: -10; reveal: 0.82; size: 0.78; tint: "#479e55" }
            ListElement { nx: 0.82; ny: 0.945; angle: 100; reveal: 0.88; size: 1.08; tint: "#68c86e" }
            ListElement { nx: 0.59; ny: 0.947; angle: 274; reveal: 0.92; size: 0.80; tint: "#4ca95d" }
            ListElement { nx: 0.35; ny: 0.945; angle: 92; reveal: 0.96; size: 1.00; tint: "#61bf67" }
            ListElement { nx: 0.15; ny: 0.943; angle: 263; reveal: 0.985; size: 0.75; tint: "#459f55" }
        }

        delegate: Leaf {
            required property real nx
            required property real ny
            required property real angle
            required property real reveal
            required property real size
            required property string tint

            x: frame.width * nx
            y: frame.height * ny
            restingAngle: angle
            scale: size * Math.max(0, Math.min(1, (frame.progress - reveal) * 18))
            opacity: Math.max(0, Math.min(1, (frame.progress - reveal) * 18))
            leafColor: tint
            swayEnabled: frame.motionEnabled

            Behavior on opacity { NumberAnimation { duration: 180 } }
        }
    }

    Repeater {
        model: ListModel {
            ListElement { nx: 0.04; ny: 0.23; reveal: 0.24 }
            ListElement { nx: 0.36; ny: 0.052; reveal: 0.43 }
            ListElement { nx: 0.948; ny: 0.42; reveal: 0.73 }
            ListElement { nx: 0.49; ny: 0.948; reveal: 0.94 }
        }

        delegate: Item {
            required property real nx
            required property real ny
            required property real reveal

            x: frame.width * nx - 7
            y: frame.height * ny - 7
            width: 14
            height: 14
            opacity: Math.max(0, Math.min(1, (frame.progress - reveal) * 16))
            scale: opacity

            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "#d2d77d"
                border.color: "#617f48"
                border.width: 1
            }

            Rectangle {
                anchors.centerIn: parent
                width: 4
                height: 4
                radius: 2
                color: "#fff0a3"
            }
        }
    }
}
