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

    GridLayout {
        anchors.fill: parent
        rows: 2
        columns: 2
        columnSpacing: 10

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

            trackPolyline.path.push(QtPositioning.coordinate(latitude, longitude));
            map.center = QtPositioning.coordinate(latitude, longitude);
        }
    }
}
