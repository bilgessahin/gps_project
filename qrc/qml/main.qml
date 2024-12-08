import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick.Layouts 1.15


ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "GPS Ground Tracker"

    // Harita ve Kontroller için GridLayout
    GridLayout {
        anchors.fill: parent
        rows: 2
        columns: 2
        columnSpacing: 10

        // Harita
        Map {
            id: map
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            plugin: Plugin { name: "osm" }
            center: QtPositioning.coordinate(39.9682, 32.5488)
            zoomLevel: 8

            MapPolyline {
                id: trackPolyline
                line.width: 2
                line.color: "blue"
                path: []
            }
        }

        // Latitude ve Longitude Bilgileri
        Label {
            text: "Latitude:"
            Layout.alignment: Qt.AlignLeft
        }
        Label {
            id: latitudeLabel
            text: "N/A"
            Layout.alignment: Qt.AlignRight
        }

        Label {
            text: "Longitude:"
            Layout.alignment: Qt.AlignLeft
        }
        Label {
            id: longitudeLabel
            text: "N/A"
            Layout.alignment: Qt.AlignRight
        }

        Label {
            text: "Speed (km/h):"
            Layout.alignment: Qt.AlignLeft
        }
        Label {
            id: speedLabel
            text: "N/A"
            Layout.alignment: Qt.AlignRight
        }

        Label {
            text: "Track Head:"
            Layout.alignment: Qt.AlignLeft
        }
        Label {
            id: trackHeadLabel
            text: "N/A"
            Layout.alignment: Qt.AlignRight
        }

        // Başlat/Durdur Butonları
        Button {
            text: "Start"
            onClicked: gpsProcessor.startProcessing()
        }
        Button {
            text: "Stop"
            onClicked: gpsProcessor.stopProcessing()
        }

        Button {
            text: "Enable/Disable Tracking"
            onClicked: gpsProcessor.toggleTracking(!gpsProcessor.trackingEnabled)
        }
    }

    Connections {
        target: gpsProcessor
        function onGpsDataUpdated(latitude, longitude, speed, trackHead) {
            latitudeLabel.text = latitude.toFixed(6) + "°";
            longitudeLabel.text = longitude.toFixed(6) + "°";
            speedLabel.text = speed.toFixed(2) + " km/h";
            trackHeadLabel.text = trackHead.toFixed(2) + "°";

            // Harita Güncellemesi
            trackPolyline.path.push(QtPositioning.coordinate(latitude, longitude));
            map.center = QtPositioning.coordinate(latitude, longitude);
        }
    }
}


// ApplicationWindow {
//     visible: true
//     width: 800
//     height: 600
//     title: "GPS Ground Tracker"

//     GridLayout {
//         anchors.fill: parent
//         rows: 1
//         columns: 2
//         columnSpacing: 10
//         rowSpacing: 10

//         // Harita
//         Map {
//             id: map
//             Layout.preferredWidth: parent.width * 0.7
//             Layout.preferredHeight: parent.height
//             plugin: Plugin {
//                 name: "osm"
//                 PluginParameter {
//                     name: "osm.mapping.providersrepository.address"
//                     value: "https://tile.openstreetmap.org/"
//                 }
//                 PluginParameter {
//                     name: "osm.mapping.providersrepository.enabled"
//                     value: true
//                 }
//             }

//             center: QtPositioning.coordinate(39.9682, 32.5488)
//             zoomLevel: 8

//             MapPolyline {
//                 id: trackPolyline
//                 line.width: 2
//                 line.color: "blue"
//                 path: []
//             }
//         }

//         // Kontrol Paneli
//         GridLayout {
//             Layout.alignment: Qt.AlignRight | Qt.AlignTop
//             Layout.preferredWidth: parent.width * 0.3
//             Layout.preferredHeight: parent.height
//             columns: 2
//             rowSpacing: 20
//             columnSpacing: 20

//             // Latitude ve Longitude Alanı
//             Rectangle {
//                 color: "white"
//                 border.color: "black"
//                 border.width: 1
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 60

//                 Label {
//                     text: "Latitude:";
//                     font.pixelSize: 14;
//                     anchors.verticalCenter: parent.verticalCenter
//                 }
//             }

//             Rectangle {
//                 color: "white"
//                 border.color: "black"
//                 border.width: 1
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 60
//                 Label { id: latitudeField; text: "39.9682 deg"; font.pixelSize: 14 }

//             }

//             Rectangle {
//                 color: "white"
//                 border.color: "black"
//                 border.width: 1
//                 radius: 5
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 60

//                 Column {
//                     anchors.fill: parent
//                     anchors.margins: 5
//                     spacing: 5

//                     Label { text: "Longitude:"; font.pixelSize: 14 }
//                     Label { id: longitudeField; text: "32.5488 deg"; font.pixelSize: 14 }
//                 }
//             }

//             // Speed ve Track Head Alanı
//             Rectangle {
//                 color: "white"
//                 border.color: "black"
//                 border.width: 1
//                 radius: 5
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 60

//                 Column {
//                     anchors.fill: parent
//                     anchors.margins: 5
//                     spacing: 5

//                     Label { text: "Speed:"; font.pixelSize: 14 }
//                     Label { id: speedField; text: "120.0 km/h"; font.pixelSize: 14 }
//                 }
//             }

//             Rectangle {
//                 color: "white"
//                 border.color: "black"
//                 border.width: 1
//                 radius: 5
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 60

//                 Column {
//                     anchors.fill: parent
//                     anchors.margins: 5
//                     spacing: 5

//                     Label { text: "Track Head:"; font.pixelSize: 14 }
//                     Label { id: trackHeadField; text: "270 deg"; font.pixelSize: 14 }
//                 }
//             }

//             // Düğmeler Alanı
//             Rectangle {
//                 color: "white"
//                 border.color: "black"
//                 border.width: 1
//                 radius: 5
//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 80

//                 RowLayout {
//                     anchors.fill: parent
//                     spacing: 10

//                     Button {
//                         text: "Enable/Disable Track"
//                         onClicked:  gpsProcessor.loadFile("/home/bilge/Desktop/GpsApp/gps_nmea_out.txt")
//                     }
//                     Button {
//                         text: "Start/Stop Button"
//                         onClicked: gpsProcessor.startProcessing();
//                     }
//                 }
//             }
//         }
//     }

//     Connections {
//         target: gpsProcessor
//         function onGpsDataUpdated(latitude, longitude, speed, trackHead) {
//             console.log("latitude ", latitude);
//             console.log("longitude ", longitude);
//             console.log("speed ", speed);
//             console.log("trackHead ", trackHead);

//             latitudeField.text = latitude.toFixed(6) + " deg";
//             longitudeField.text = longitude.toFixed(6) + " deg";
//             speedField.text = speed.toFixed(2) + " km/h";
//             trackHeadField.text = trackHead.toFixed(2) + "°";

//             trackPolyline.path.push(QtPositioning.coordinate(latitude, longitude));
//             map.center = QtPositioning.coordinate(latitude, longitude);
//         }
//     }
// }


