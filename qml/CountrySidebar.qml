import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    color: "#111128"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── Header ────────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#0d0d1a"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 16
                spacing: 10

                Text {
                    text: "\uD83C\uDF0D"  // 🌍
                    font.pixelSize: 18
                }
                Text {
                    text: "Countries"
                    color: "#a78bfa"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    font.letterSpacing: 1.5
                }
            }

            // bottom border
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#2d2d5e"
            }
        }

        // ── Country list ──────────────────────────────────────────────────
        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: radioApp.countries
            clip: true

            delegate: Item {
                width: list.width
                height: 72

                // selection / hover background
                Rectangle {
                    anchors.fill: parent
                    color: radioApp.selectedCountryId === model.id
                           ? "#1e1e4a"
                           : (hoverArea.containsMouse ? "#161636" : "transparent")

                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                // left accent bar (selected)
                Rectangle {
                    width: 4
                    height: parent.height
                    color: "#7c3aed"
                    opacity: radioApp.selectedCountryId === model.id ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 200 } }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 16
                    spacing: 14

                    Text {
                        text: model.flag
                        font.pixelSize: 28
                    }

                    Column {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            width: parent.width
                            text: model.name
                            color: radioApp.selectedCountryId === model.id ? "#e2e8f0" : "#94a3b8"
                            font.pixelSize: 15
                            font.weight: radioApp.selectedCountryId === model.id ? Font.Medium : Font.Normal
                            elide: Text.ElideRight

                            Behavior on color { ColorAnimation { duration: 150 } }
                        }

                        Text {
                            text: model.code
                            color: "#4a4a7a"
                            font.pixelSize: 11
                            font.letterSpacing: 1
                        }
                    }

                    // playing indicator arrow
                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: "#22c55e"
                        opacity: radioApp.selectedCountryId === model.id &&
                                 radioApp.player.playing ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: 200 } }
                    }
                }

                MouseArea {
                    id: hoverArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: radioApp.selectCountry(model.id, model.name, model.flag)
                }
            }

            // scroll indicator
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
