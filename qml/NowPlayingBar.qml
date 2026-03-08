import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    height: 130   // 52 spectrum + 1 divider + 77 controls
    color:  "#090916"

    // ── Layout: spectrum strip on top, controls row below ─────────────────
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Spectrum analyzer strip ───────────────────────────────────────
        SpectrumAnalyzer {
            Layout.fillWidth:    true
            Layout.preferredHeight: 52
            active:   radioApp.player.playing
            loading:  radioApp.player.loading
            barCount: 36
            gap:      2

            // Fade in/out with playback state
            opacity: (radioApp.player.playing || radioApp.player.loading) ? 1.0 : 0.18
            Behavior on opacity { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }
        }

        // ── Thin divider ──────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth:    true
            Layout.preferredHeight: 1
            color: "#1e1e3f"
        }

        // ── Controls row ──────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth:    true
            Layout.fillHeight:   true
            Layout.leftMargin:   24
            Layout.rightMargin:  24
            Layout.topMargin:    10
            Layout.bottomMargin: 10
            spacing: 20

            // ── Signal rings (loading only) ───────────────────────────────
            Item {
                width:  28
                height: 28
                visible: radioApp.player.loading

                Repeater {
                    model: 3
                    Rectangle {
                        visible: radioApp.player.loading
                        anchors.centerIn: parent
                        width:  6 + index * 8
                        height: width
                        radius: width / 2
                        color:  "transparent"
                        border.color: "#7c3aed"
                        border.width: 1.5

                        SequentialAnimation on opacity {
                            running: radioApp.player.loading
                            loops:   Animation.Infinite
                            PauseAnimation    { duration: index * 250 }
                            NumberAnimation   { from: 0.9; to: 0.0; duration: 900; easing.type: Easing.OutCubic }
                            PauseAnimation    { duration: (2 - index) * 250 }
                        }
                        SequentialAnimation on scale {
                            running: radioApp.player.loading
                            loops:   Animation.Infinite
                            PauseAnimation    { duration: index * 250 }
                            NumberAnimation   { from: 0.5; to: 1.4; duration: 900; easing.type: Easing.OutCubic }
                            PauseAnimation    { duration: (2 - index) * 250 }
                        }
                    }
                }
            }

            // Placeholder so layout doesn't shift when rings hide
            Item {
                width:  28
                height: 28
                visible: !radioApp.player.loading
            }

            // ── Station info ──────────────────────────────────────────────
            Column {
                Layout.fillWidth: true
                spacing: 3

                readonly property bool hasError: radioApp.player.errorStation.length > 0
                                                 && !radioApp.player.playing
                                                 && !radioApp.player.loading

                Text {
                    width: parent.width
                    text:  radioApp.player.stationName.length > 0
                           ? radioApp.player.stationName
                           : (parent.hasError ? radioApp.player.errorStation : "Not playing")
                    color: parent.hasError   ? "#fca5a5"
                         : (radioApp.player.playing || radioApp.player.loading) ? "#e2e8f0"
                         : "#4a4a7a"
                    font.pixelSize: 15
                    font.weight:    (radioApp.player.playing || radioApp.player.loading) ? Font.Medium : Font.Normal
                    elide: Text.ElideRight

                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                Row {
                    spacing: 5
                    visible: radioApp.player.loading

                    Repeater {
                        model: 3
                        Rectangle {
                            width:  5; height: 5; radius: 2.5; color: "#7c3aed"
                            SequentialAnimation on opacity {
                                running: radioApp.player.loading
                                loops:   Animation.Infinite
                                PauseAnimation  { duration: index * 180 }
                                NumberAnimation { from: 0.25; to: 1.0;  duration: 350; easing.type: Easing.InOutSine }
                                NumberAnimation { from: 1.0;  to: 0.25; duration: 350; easing.type: Easing.InOutSine }
                                PauseAnimation  { duration: (2 - index) * 180 }
                            }
                        }
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Buffering stream"; color: "#94a3b8"; font.pixelSize: 12
                    }
                }

                Text {
                    width:   parent.width
                    visible: !radioApp.player.loading
                    text: parent.hasError         ? "Stream unavailable — tap to retry"
                        : radioApp.player.playing  ? radioApp.player.genre
                        :                            "Select a country, then a station"
                    color:          parent.hasError ? "#f87171" : "#94a3b8"
                    font.pixelSize: 12
                    elide:          Text.ElideRight
                }
            }

            // ── Volume control ────────────────────────────────────────────
            RowLayout {
                spacing: 6
                Text {
                    text: "\uD83D\uDD0A"; font.pixelSize: 16; color: "#94a3b8"
                }
                Slider {
                    from: 0.0; to: 1.0
                    value: radioApp.player.volume
                    implicitWidth: 110
                    Material.accent: Material.Purple
                    onMoved: radioApp.player.setVolume(value)
                }
            }

            // ── Stop button ───────────────────────────────────────────────
            Rectangle {
                visible: radioApp.player.playing || radioApp.player.loading
                width: 80; height: 40; radius: 8
                color: stopMouse.containsMouse ? "#7c1d1d" : "#4c1d1d"
                Behavior on color { ColorAnimation { duration: 150 } }

                Row {
                    anchors.centerIn: parent; spacing: 6
                    Text { anchors.verticalCenter: parent.verticalCenter
                           text: "\u25A0"; color: "#fca5a5"; font.pixelSize: 13 }
                    Text { anchors.verticalCenter: parent.verticalCenter
                           text: "Stop"; color: "#fca5a5"; font.pixelSize: 13; font.weight: Font.Medium }
                }
                MouseArea {
                    id: stopMouse; anchors.fill: parent
                    hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: radioApp.player.stop()
                }
            }
        }
    }
}
