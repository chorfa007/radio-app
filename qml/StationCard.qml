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

    radius: 12
    color:         isPlaying ? "#130d2e" : (cardMouse.containsMouse ? "#22223d" : "#1a1a35")
    border.color:  isPlaying ? "#7c3aed" : (cardMouse.containsMouse ? "#3d3d70" : "#2a2a50")
    border.width:  isPlaying ? 2 : 1

    Behavior on color        { ColorAnimation { duration: 180 } }
    Behavior on border.color { ColorAnimation { duration: 180 } }

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

    ColumnLayout {
        anchors.fill:    parent
        anchors.margins: 16
        spacing: 8

        // ── Logo + Name row ───────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Station logo
            Rectangle {
                width:  40
                height: 40
                radius: 8
                color:  "#0d0d2a"

                Image {
                    id: logoImg
                    anchors.fill:    parent
                    anchors.margins: 4
                    source:          card.logoUrl
                    fillMode:        Image.PreserveAspectFit
                    smooth:          true
                    visible:         status === Image.Ready
                }

                // Fallback: first letter when logo not loaded
                Text {
                    anchors.centerIn: parent
                    visible:          logoImg.status !== Image.Ready
                    text:             card.stationName.length > 0 ? card.stationName[0].toUpperCase() : "?"
                    color:            "#7c3aed"
                    font.pixelSize:   18
                    font.weight:      Font.Bold
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                // Animated live dot + name
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Rectangle {
                        width:  7
                        height: 7
                        radius: 3.5
                        color:  "#22c55e"
                        visible: isPlaying

                        SequentialAnimation on opacity {
                            running: isPlaying
                            loops:   Animation.Infinite
                            NumberAnimation { to: 0.2; duration: 700; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 700; easing.type: Easing.InOutSine }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text:  card.stationName
                        color: isPlaying ? "#c4b5fd" : "#e2e8f0"
                        font.pixelSize: 14
                        font.weight:    Font.Medium
                        elide: Text.ElideRight

                        Behavior on color { ColorAnimation { duration: 180 } }
                    }
                }
            }
        }

        // ── Genre badge ───────────────────────────────────────────────────
        Rectangle {
            radius: 4
            color:  "#1e0f3d"
            height: genreLbl.implicitHeight + 6
            width:  genreLbl.implicitWidth  + 14

            Text {
                id: genreLbl
                anchors.centerIn: parent
                text:  card.genre
                color: "#a78bfa"
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
        }

        Item { Layout.fillHeight: true }

        // ── Play / Stop button ────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 36
            radius: 8
            color:  isPlaying
                    ? (cardMouse.containsMouse ? "#6d28d9" : "#5b21b6")
                    : (cardMouse.containsMouse ? "#3b1f7a" : "#2d1b5e")

            Behavior on color { ColorAnimation { duration: 150 } }

            Row {
                anchors.centerIn: parent
                spacing: 8

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text:           isPlaying ? "\u25A0" : "\u25B6"  // ■ or ▶
                    color:          isPlaying ? "#ffffff" : "#a78bfa"
                    font.pixelSize: 13
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text:           isPlaying ? "Stop" : "Play"
                    color:          isPlaying ? "#ffffff" : "#a78bfa"
                    font.pixelSize: 13
                    font.weight:    Font.Medium
                }
            }
        }
    }
}
