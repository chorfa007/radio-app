import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width:       1100
    height:      700
    minimumWidth:  900
    minimumHeight: 600
    visible: true
    title: "Radio App"

    Material.theme:      Material.Dark
    Material.accent:     Material.Purple
    Material.background: "#0d0d1a"

    background: Rectangle { color: "#0d0d1a" }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth:  true
            Layout.fillHeight: true
            spacing: 0

            // ── Country sidebar ──────────────────────────────────────────
            CountrySidebar {
                Layout.preferredWidth: 230
                Layout.fillHeight: true
            }

            // ── Divider ──────────────────────────────────────────────────
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                color: "#2d2d5e"
            }

            // ── Station grid ─────────────────────────────────────────────
            RadioGrid {
                Layout.fillWidth:  true
                Layout.fillHeight: true
            }
        }

        // ── Now playing bar ───────────────────────────────────────────────
        NowPlayingBar {
            Layout.fillWidth: true
        }
    }
}
