import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    color: "#0d0d1a"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Header bar ────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#0d0d1a"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                spacing: 10

                Text {
                    text: radioApp.selectedCountryFlag
                    font.pixelSize: 24
                    visible: radioApp.selectedCountryId !== -1
                }

                Text {
                    text: radioApp.selectedCountryId === -1
                          ? "Radio App"
                          : radioApp.selectedCountryName
                    color: "#e2e8f0"
                    font.pixelSize: 18
                    font.weight: Font.Medium
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: radioApp.radios.count + " stations"
                    color: "#94a3b8"
                    font.pixelSize: 12
                    visible: radioApp.selectedCountryId !== -1
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#2d2d5e"
            }
        }

        // ── Empty state ───────────────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: radioApp.selectedCountryId === -1

            Column {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "\uD83D\uDCFB"  // 📻
                    font.pixelSize: 72
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Pick a country to browse stations"
                    color: "#94a3b8"
                    font.pixelSize: 17
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Tunisia  \u00B7  Belgium  \u00B7  United Kingdom  \u00B7  France"
                    color: "#4a4a7a"
                    font.pixelSize: 13
                }
            }
        }

        // ── Station grid ──────────────────────────────────────────────────
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: radioApp.selectedCountryId !== -1
            clip: true

            GridView {
                id: grid
                width: parent.width

                readonly property int minCardWidth: 260
                readonly property int cols: Math.max(1, Math.floor(width / minCardWidth))

                cellWidth:  Math.floor(width / cols)
                cellHeight: 185

                topMargin:    16
                bottomMargin: 16
                leftMargin:   8
                rightMargin:  8

                model: radioApp.radios

                delegate: Item {
                    width:  grid.cellWidth
                    height: grid.cellHeight

                    StationCard {
                        anchors {
                            fill:    parent
                            margins: 8
                        }
                        stationName:  model.stationName
                        genre:        model.genre
                        description:  model.description
                        streamUrl:    model.streamUrl
                        logoUrl:      model.logoUrl
                        isPlaying:    radioApp.player.playing &&
                                      radioApp.player.stationName === model.stationName
                    }
                }
            }
        }
    }
}
