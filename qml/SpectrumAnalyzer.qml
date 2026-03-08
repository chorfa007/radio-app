import QtQuick

// Winamp-style spectrum analyzer drawn on a Canvas.
// Bars: green (low) → yellow (mid) → red (high), with white peak dots.
Canvas {
    id: root

    property bool   active:   false   // true = playing, animate fully
    property bool   loading:  false   // true = connecting, gentle idle
    property int    barCount: 28
    property real   gap:      2

    // ── Internal state (plain JS – no QML property overhead) ─────────────
    property var _h: []   // current bar heights
    property var _p: []   // peak heights
    property var _t: []   // target heights

    Component.onCompleted: _init()

    function _init() {
        var h = [], p = [], t = []
        for (var i = 0; i < barCount; i++) { h.push(1); p.push(1); t.push(0) }
        _h = h; _p = p; _t = t
    }

    // ── Animation timer ───────────────────────────────────────────────────
    Timer {
        interval: 38   // ~26 fps
        running:  true
        repeat:   true

        onTriggered: {
            if (root._h.length !== root.barCount) { root._init(); return }

            var maxH = root.height - 6
            var h = root._h, p = root._p, t = root._t
            var n = root.barCount

            for (var i = 0; i < n; i++) {
                if (root.active) {
                    // Randomise target with a bell-curve frequency profile
                    // (mid-bass peaks higher, high freq tapers off)
                    if (Math.random() < 0.28) {
                        var pos   = (i + 0.5) / n                  // 0..1
                        var shape = Math.sin(Math.PI * pos) * 0.7  // bell
                              + Math.sin(Math.PI * pos * 2) * 0.2  // harmonics
                              + Math.random() * 0.3                 // noise
                        t[i] = shape * maxH
                    }
                } else if (root.loading) {
                    // Gentle low-energy idle
                    if (Math.random() < 0.12) {
                        t[i] = Math.random() * maxH * 0.22
                    }
                } else {
                    // Decay to floor
                    t[i] = Math.max(0, t[i] - 4)
                }

                // Exponential smoothing toward target
                h[i] = h[i] + (t[i] - h[i]) * 0.38
                h[i] = Math.max(1, h[i])

                // Peak: follow up instantly, fall slowly
                if (h[i] >= p[i]) {
                    p[i] = h[i]
                } else {
                    p[i] = Math.max(1, p[i] - 1.4)
                }
            }

            root._h = h; root._p = p; root._t = t
            root.requestPaint()
        }
    }

    // ── Painting ─────────────────────────────────────────────────────────
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        if (_h.length !== barCount) return

        var n    = barCount
        var g    = gap
        var barW = Math.max(2, (width - g * (n - 1)) / n)
        var off  = (width - (barW * n + g * (n - 1))) / 2
        var maxH = height - 6   // 3px peak zone + 3px padding

        for (var i = 0; i < n; i++) {
            var x = off + i * (barW + g)
            var h = Math.max(1, _h[i])
            var y = height - h - 3

            // Classic Winamp gradient: green → yellow → red (bottom → top)
            var grad = ctx.createLinearGradient(0, height - 3, 0, 0)
            grad.addColorStop(0.00, "#00e676")   // bright green
            grad.addColorStop(0.55, "#ffeb3b")   // yellow
            grad.addColorStop(0.80, "#ff5722")   // orange
            grad.addColorStop(1.00, "#f44336")   // red

            ctx.fillStyle = grad
            ctx.fillRect(x, y, barW, h)

            // White peak dot (2 px tall)
            var py = Math.max(0, height - _p[i] - 5)
            ctx.fillStyle = "rgba(255,255,255,0.88)"
            ctx.fillRect(x, py, barW, 2)
        }
    }
}
