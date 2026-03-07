import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    height: 76
    color:  "#090916"

    // top border
    Rectangle {
        width:  parent.width
        height: 1
        color:  "#2d2d5e"
    }

    RowLayout {
        anchors.fill:         parent
        anchors.leftMargin:   24
        anchors.rightMargin:  24
        anchors.topMargin:    10
        anchors.bottomMargin: 10
        spacing: 20

        // ── Waveform (playing) / Signal rings (loading) ───────────────────
        Item {
            width:  28
            height: 28
            visible: radioApp.player.playing || radioApp.player.loading

            // Animated waveform bars — visible when playing
            Row {
                anchors.centerIn: parent
                spacing: 3
                visible: radioApp.player.playing

                Repeater {
                    model: 4
                    Rectangle {
                        width:  4
                        radius: 2
                        color:  "#7c3aed"
                        anchors.verticalCenter: parent ? parent.verticalCenter : undefined

                        readonly property real baseHeight: 14
                        height: baseHeight

                        SequentialAnimation on height {
                            running: radioApp.player.playing
                            loops:   Animation.Infinite
                            NumberAnimation {
                                to:       baseHeight * (0.4 + Math.random() * 0.6)
                                duration: 300 + index * 80
                                easing.type: Easing.InOutSine
                            }
                            NumberAnimation {
                                to:       baseHeight
                                duration: 300 + index * 80
                                easing.type: Easing.InOutSine
                            }
                        }
                    }
                }
            }

            // Expanding signal rings — visible when connecting
            Repeater {
                model: 3
                visible: radioApp.player.loading

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

        // ── Station info ──────────────────────────────────────────────────
        Column {
            Layout.fillWidth: true
            spacing: 3

            Text {
                width: parent.width
                text:  radioApp.player.stationName.length > 0
                       ? radioApp.player.stationName
                       : "Not playing"
                color: (radioApp.player.playing || radioApp.player.loading) ? "#e2e8f0" : "#4a4a7a"
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
                        width:  5
                        height: 5
                        radius: 2.5
                        color:  "#7c3aed"

                        SequentialAnimation on opacity {
                            running: radioApp.player.loading
                            loops:   Animation.Infinite
                            PauseAnimation  { duration: index * 180 }
                            NumberAnimation { from: 0.25; to: 1.0; duration: 350; easing.type: Easing.InOutSine }
                            NumberAnimation { from: 1.0;  to: 0.25; duration: 350; easing.type: Easing.InOutSine }
                            PauseAnimation  { duration: (2 - index) * 180 }
                        }
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text:           "Buffering stream"
                    color:          "#94a3b8"
                    font.pixelSize: 12
                }
            }

            Text {
                width:   parent.width
                visible: !radioApp.player.loading
                text: radioApp.player.playing ? radioApp.player.genre
                                              : "Select a country, then a station"
                color:          "#94a3b8"
                font.pixelSize: 12
                elide:          Text.ElideRight
            }
        }

        // ── Volume control ────────────────────────────────────────────────
        RowLayout {
            spacing: 6

            Text {
                text:           "\uD83D\uDD0A"  // 🔊
                font.pixelSize: 16
                color:          "#94a3b8"
            }

            Slider {
                id: volSlider
                from:  0.0
                to:    1.0
                value: radioApp.player.volume
                implicitWidth: 110
                Material.accent: Material.Purple

                onMoved: radioApp.player.setVolume(value)
            }
        }

        // ── Stop button ───────────────────────────────────────────────────
        Rectangle {
            visible: radioApp.player.playing || radioApp.player.loading
            width:   80
            height:  40
            radius:  8
            color:   stopMouse.containsMouse ? "#7c1d1d" : "#4c1d1d"

            Behavior on color { ColorAnimation { duration: 150 } }

            Row {
                anchors.centerIn: parent
                spacing: 6

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text:           "\u25A0"  // ■
                    color:          "#fca5a5"
                    font.pixelSize: 13
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text:           "Stop"
                    color:          "#fca5a5"
                    font.pixelSize: 13
                    font.weight:    Font.Medium
                }
            }

            MouseArea {
                id: stopMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape:  Qt.PointingHandCursor
                onClicked:    radioApp.player.stop()
            }
        }
    }
}
