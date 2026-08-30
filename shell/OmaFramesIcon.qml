import QtQuick

Item {
  id: root

  property real iconSize: 18
  property color color: "white"
  property bool loaded: false
  property bool active: false

  width: iconSize
  height: iconSize

  onColorChanged: canvas.requestPaint()
  onLoadedChanged: canvas.requestPaint()
  onActiveChanged: canvas.requestPaint()
  onIconSizeChanged: canvas.requestPaint()

  Canvas {
    id: canvas
    anchors.fill: parent
    antialiasing: true

    onPaint: {
      var ctx = getContext("2d")
      var scale = Math.min(width, height) / 18
      ctx.clearRect(0, 0, width, height)
      ctx.save()
      ctx.scale(scale, scale)
      ctx.strokeStyle = root.color
      ctx.fillStyle = root.color
      ctx.globalAlpha = root.loaded ? 1.0 : 0.58
      ctx.lineCap = "round"
      ctx.lineJoin = "round"
      ctx.lineWidth = 1.7

      // A compact sprout: the stem suggests Vines while the curled tail gives
      // the silhouette a little Chameleon personality at bar-icon size.
      ctx.beginPath()
      ctx.moveTo(8.9, 15.2)
      ctx.bezierCurveTo(8.7, 11.8, 9.3, 8.7, 11.7, 6.0)
      ctx.bezierCurveTo(13.1, 4.5, 14.7, 4.8, 14.8, 6.1)
      ctx.bezierCurveTo(14.9, 7.2, 13.9, 7.9, 13.1, 7.4)
      ctx.stroke()

      ctx.beginPath()
      ctx.moveTo(9.2, 11.3)
      ctx.bezierCurveTo(6.3, 10.9, 4.6, 9.1, 4.4, 6.6)
      ctx.stroke()

      // Paired heart-like leaves stay legible without relying on a font glyph.
      ctx.beginPath()
      ctx.moveTo(8.0, 10.0)
      ctx.bezierCurveTo(5.7, 10.0, 3.9, 8.6, 4.0, 6.0)
      ctx.bezierCurveTo(6.5, 5.8, 8.0, 7.4, 8.0, 10.0)
      ctx.fill()

      ctx.beginPath()
      ctx.moveTo(10.4, 8.0)
      ctx.bezierCurveTo(10.4, 5.5, 12.0, 3.8, 14.6, 4.0)
      ctx.bezierCurveTo(14.4, 6.4, 12.8, 7.8, 10.4, 8.0)
      ctx.fill()

      if (root.loaded) {
        ctx.globalAlpha = root.active ? 1.0 : 0.52
        ctx.beginPath()
        ctx.arc(14.7, 14.4, 1.35, 0, Math.PI * 2)
        ctx.fill()
      }
      ctx.restore()
    }
  }
}
