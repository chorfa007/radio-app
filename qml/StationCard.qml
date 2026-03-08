import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: card

    property string stationName: ""
    property string genre:       ""
    property string description: ""
    property string streamUrl:   ""
    property string logoUrl:     ""
    property bool   isPlaying:   false

    readonly property bool hasError: !isPlaying
                                     && radioApp.player.errorStation === card.stationName

    // ── Glow pulse driver (0 → 1 → 0, loops while playing) ───────────────
    property real _glow: 0.0
    SequentialAnimation on _glow {
        running: isPlaying
        loops:   Animation.Infinite
        NumberAnimation { to: 1.0; duration: 950; easing.type: Easing.InOutSine }
        NumberAnimation { to: 0.0; duration: 950; easing.type: Easing.InOutSine }
    }
    onIsPlayingChanged: if (!isPlaying) _glow = 0

    // ── Card shape ────────────────────────────────────────────────────────
    radius: 12

    // Pop out slightly when playing
    scale: isPlaying ? 1.04 : 1.0
    Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutBack } }

    // Background — unchanged when playing
    color: hasError   ? "#1a0d0d"
         : cardMouse.containsMouse ? "#22223d" : "#1a1a35"

    // Border pulses green while playing
    border.color: hasError  ? "#7f1d1d"
                : isPlaying ? Qt.rgba(0.13 + _glow * 0.21,
                                      0.77 + _glow * 0.23,
                                      0.37 + _glow * 0.20, 1.0)
                : cardMouse.containsMouse ? "#3d3d70" : "#2a2a50"
    border.width: isPlaying ? (2.0 + _glow * 2.0) : (hasError ? 2 : 1)

    Behavior on color { ColorAnimation { duration: 180 } }

    // ── "NOW PLAYING" badge (top-right) ───────────────────────────────────
    Rectangle {
        visible: isPlaying
        anchors { top: parent.top; right: parent.right; topMargin: 10; rightMargin: 10 }
        radius: 6
        color:  Qt.rgba(0.08 + _glow * 0.18, 0.55 + _glow * 0.22, 0.24 + _glow * 0.16, 0.90)
        width:  nowLbl.implicitWidth + 12
        height: nowLbl.implicitHeight + 6
        z: 10

        Text {
            id: nowLbl
            anchors.centerIn: parent
            text:             "NOW PLAYING"
            color:            "#ffffff"
            font.pixelSize:   8
            font.weight:      Font.Bold
            font.letterSpacing: 1.2
        }
    }

    // ── Mouse area ────────────────────────────────────────────────────────
    MouseArea {
        id: cardMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape:  Qt.PointingHandCursor
        onClicked: {
            if (card.isPlaying)
                radioApp.player.stop()
            else
                radioApp.player.play(card.streamUrl, card.stationName,
                                     card.genre, card.description)
        }
    }

    // ── Content ───────────────────────────────────────────────────────────
    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 16
        spacing: 8

        // ── Logo + Name row ───────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Station logo with pulsing ring when playing
            Item {
                width:  40
                height: 40

                // Glowing green ring behind logo
                Rectangle {
                    visible: isPlaying
                    anchors.centerIn: parent
                    width:  parent.width  + 8 + _glow * 6
                    height: parent.height + 8 + _glow * 6
                    radius: width / 2
                    color:  "transparent"
                    border.color: Qt.rgba(0.13 + _glow * 0.21, 0.77 + _glow * 0.23, 0.37 + _glow * 0.20, 0.55 + _glow * 0.35)
                    border.width: 1.5
                    Behavior on width  { NumberAnimation { duration: 50 } }
                    Behavior on height { NumberAnimation { duration: 50 } }
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color:  logoImg.status === Image.Ready ? "#0d0d2a" : avatarColor()

                    function avatarColor() {
                        var palette = ["#4c1d95","#1e3a5f","#064e3b","#7c2d12",
                                       "#1e1b4b","#134e4a","#3b0764","#1c1917"]
                        var hash = 0
                        for (var i = 0; i < card.stationName.length; i++)
                            hash = (hash * 31 + card.stationName.charCodeAt(i)) & 0xffff
                        return palette[hash % palette.length]
                    }

                    Image {
                        id: logoImg
                        anchors.fill:    parent
                        anchors.margins: 4
                        source:          card.logoUrl
                        fillMode:        Image.PreserveAspectFit
                        smooth:          true
                        visible:         status === Image.Ready
                    }

                    Text {
                        anchors.centerIn: parent
                        visible:          logoImg.status !== Image.Ready
                        text:             card.stationName.length > 0 ? card.stationName[0].toUpperCase() : "?"
                        color:            "#ffffff"
                        font.pixelSize:   18
                        font.weight:      Font.Bold
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    // Live dot
                    Rectangle {
                        width:  7; height: 7; radius: 3.5
                        color:  "#22c55e"
                        visible: isPlaying
                        SequentialAnimation on opacity {
                            running: isPlaying; loops: Animation.Infinite
                            NumberAnimation { to: 0.2; duration: 700; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 700; easing.type: Easing.InOutSine }
                        }
                    }

                    // Error dot
                    Rectangle {
                        width: 7; height: 7; radius: 3.5
                        color: "#ef4444"; visible: hasError
                    }

                    Text {
                        Layout.fillWidth: true
                        text:  card.stationName
                        color: hasError ? "#fca5a5" : "#e2e8f0"
                        font.pixelSize: 14
                        font.weight:    Font.Medium
                        elide: Text.ElideRight
                        Behavior on color         { ColorAnimation  { duration: 180 } }
                        Behavior on font.pixelSize{ NumberAnimation { duration: 150 } }
                    }
                }
            }
        }

        // ── Genre badge ───────────────────────────────────────────────────
        Rectangle {
            radius: 4
            color:  hasError ? "#3d0f0f" : "#1e0f3d"
            height: genreLbl.implicitHeight + 6
            width:  genreLbl.implicitWidth  + 14

            Text {
                id: genreLbl
                anchors.centerIn: parent
                text:  card.genre
                color: hasError ? "#f87171" : "#a78bfa"
                font.pixelSize: 11
            }
        }

        // ── Description ───────────────────────────────────────────────────
        Text {
            Layout.fillWidth: true
            text:             card.description
            color:            "#94a3b8"
            font.pixelSize:   12
            wrapMode:         Text.WordWrap
            maximumLineCount: 2
            elide:            Text.ElideRight
            Behavior on color { ColorAnimation { duration: 180 } }
        }

        Item { Layout.fillHeight: true }

        // ── Play / Stop / Unavailable button ─────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 36
            radius: 8
            color: hasError
                   ? (cardMouse.containsMouse ? "#7f1d1d" : "#450a0a")
                   : isPlaying
                     ? (cardMouse.containsMouse ? "#6d28d9" : "#5b21b6")
                     : (cardMouse.containsMouse ? "#3b1f7a" : "#2d1b5e")

            Behavior on color { ColorAnimation { duration: 150 } }

            Row {
                anchors.centerIn: parent
                spacing: 8

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: hasError  ? "\u26A0"
                        : isPlaying ? "\u25A0"
                        :             "\u25B6"
                    color: hasError  ? "#fca5a5"
                         : isPlaying ? "#ffffff" : "#a78bfa"
                    font.pixelSize: 13
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: hasError  ? "Stream unavailable"
                        : isPlaying ? "Stop"
                        :             card.stationName
                    color: hasError  ? "#fca5a5"
                         : isPlaying ? "#ffffff" : "#a78bfa"
                    font.pixelSize: 12
                    font.weight:    Font.Medium
                    elide:          Text.ElideRight
                    maximumLineCount: 1
                    width: Math.min(implicitWidth, card.width - 80)
                }
            }
        }
    }
}
