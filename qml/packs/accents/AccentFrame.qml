import QtQuick

Canvas {
    id: root
    property string pack: "fireflies"
    property color accent: pack === "fireflies" ? "#d9f576" : pack === "frost" ? "#9ddfff" : "#e8a45b"
    property bool motionEnabled: true
    property real elapsed: 0
    property real speed: 44
    property real growth: 1
    property real inset: 20
    onElapsedChanged: requestPaint()
    onAccentChanged: requestPaint()
    onPackChanged: requestPaint()
    onGrowthChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onMotionEnabledChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        const x = inset, y = inset, w = width - inset*2, h = height - inset*2
        const perimeter = 2*(w+h), t = motionEnabled ? elapsed : 0
        function point(distance) {
            let d = ((distance % perimeter) + perimeter) % perimeter
            if (d < w) return {x: x+d, y: y}
            d -= w
            if (d < h) return {x: x+w, y: y+d}
            d -= h
            if (d < w) return {x: x+w-d, y: y+h}
            d -= w
            return {x: x, y: y+h-d}
        }
        function dot(px, py, radius, color, opacity) {
            ctx.globalAlpha = opacity
            ctx.fillStyle = color
            ctx.beginPath(); ctx.arc(px, py, radius, 0, Math.PI*2); ctx.fill()
        }
        function line(ax, ay, bx, by, color, thickness, opacity) {
            ctx.globalAlpha = opacity; ctx.strokeStyle = color; ctx.lineWidth = thickness
            ctx.beginPath(); ctx.moveTo(ax,ay); ctx.lineTo(bx,by); ctx.stroke()
        }
        if (pack === "fireflies") {
            for (let i=0; i<12; ++i) {
                const phase = i*2.399
                const pos = point(perimeter*(i+0.3)/12+t*speed*0.12)
                const px=pos.x+Math.sin(t*0.7+phase)*6, py=pos.y+Math.cos(t*0.9+phase)*6
                const glow=motionEnabled ? 0.45+0.55*Math.pow(0.5+0.5*Math.sin(t*1.5+phase),2):0.8
                for(let radius=7; radius>=2; --radius) dot(px,py,radius,accent,glow*0.035)
                dot(px,py,1.3,"#e6f7ff",glow)
            }
        } else if (pack === "frost") {
            ctx.lineCap = "round"
            const size=Math.min(128,w*0.45,h*0.45)*growth
            for(let corner=0;corner<4;++corner) {
                ctx.save()
                ctx.translate(x+(corner%2?w:0),y+(corner>1?h:0))
                ctx.scale((corner%2?-1:1)*size/128,(corner>1?-1:1)*size/128)
                for(let axis=0;axis<2;++axis) {
                    ctx.save()
                    if(axis) ctx.rotate(Math.PI/2)
                    line(0,0,96,0,accent,1.3,0.85)
                    for(let j=1;j<=7;++j) {
                        const bx=j*11, branch=(1-j/8)*13
                        for(let side=-1;side<=1;side+=2) {
                            line(bx,0,bx+branch*0.65,side*branch,"#e6f7ff",0.8,0.75)
                            line(bx+branch*0.35,side*branch*0.55,bx+branch*0.8,side*branch*0.55,accent,0.8,0.7)
                        }
                    }
                    ctx.restore()
                }
                ctx.restore()
            }
        } else {
            for(let d=5;d<perimeter;d+=13) {
                const pos=point(d), next=point(d+1)
                const horizontal=Math.abs(next.x-pos.x)>Math.abs(next.y-pos.y)
                line(pos.x-(horizontal?0:5),pos.y-(horizontal?5:0),pos.x+(horizontal?0:5),pos.y+(horizontal?5:0),accent,2,0.45)
            }
            ctx.globalAlpha=0.65; ctx.strokeStyle="#e6f7ff"; ctx.lineWidth=1
            ctx.strokeRect(x-3,y-3,w+6,h+6); ctx.strokeRect(x+3,y+3,w-6,h-6)
            for(let car=2;car>=0;--car) {
                const d=40+t*speed-car*19, pos=point(d), next=point(d+1)
                ctx.save()
                ctx.translate(pos.x,pos.y)
                ctx.rotate(Math.atan2(next.y-pos.y,next.x-pos.x))
                ctx.globalAlpha=1; ctx.fillStyle="#141f29"
                for(let axle=-4;axle<=4;axle+=8) {
                    ctx.fillRect(axle-1.5,-6,3,3); ctx.fillRect(axle-1.5,3,3,3)
                }
                ctx.fillStyle=accent; ctx.fillRect(-7.5,-4,15,8)
                if(car===0) {
                    ctx.fillRect(-7,-5,6,10)
                    ctx.fillStyle="#e6f7ff"; ctx.fillRect(-5.5,-2.5,3,5)
                    ctx.fillRect(6,-2,2,4)
                    ctx.fillStyle="#141f29"; ctx.fillRect(1.5,-2.5,3,5)
                } else {
                    ctx.fillStyle="#e6f7ff"
                    ctx.fillRect(-4.5,-2,3,4); ctx.fillRect(1.5,-2,3,4)
                }
                ctx.restore()
            }
        }
        ctx.globalAlpha=1
    }
}
