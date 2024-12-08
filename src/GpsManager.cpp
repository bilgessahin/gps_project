#include <include/GpsManager.h>
#include <include/Constants.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <cmath>
#include <QtEndian>

GpsDataProcessor::GpsDataProcessor(QObject* parent)
    : QObject(parent),
    timer(new QTimer(this)),
    udpSocket(new QUdpSocket(this)),
    currentIndex(0),
    trackingEnabled(true) {

    connect(timer, &QTimer::timeout, this, [this]() {
        if (currentIndex < gpsPackets.size()) {
            GpsData gpsData;
            if (parseGpsPacket(gpsPackets[currentIndex], gpsData)) {
                emit gpsDataUpdated(gpsData.latitude, gpsData.longitude, gpsData.speed, gpsData.trackHead);

                if (trackingEnabled) {
                    sendUdpPacket(gpsData);
                }
            }
            currentIndex++;
        } else {
            timer->stop();
            qDebug() << "Tüm GPS verileri işlendi.";
        }
    });
}

bool GpsDataProcessor::loadFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Dosya açılamadı:" << file.errorString();
        return false;
    }

    QTextStream in(&file);
    gpsPackets.clear();

    QStringList packetLines;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        packetLines.append(line);

        if (packetLines.size() == 3) {
            if (packetLines[0].startsWith("$GPGGA") &&
                packetLines[1].startsWith("$GPGSA") &&
                packetLines[2].startsWith("$GPRMC")) {

                bool valid = true;
                for (const QString& packetLine : packetLines) {
                    if (!validateNmeaString(packetLine)) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    gpsPackets.append({ packetLines[0], packetLines[1], packetLines[2] });
                }

            } else {
                qWarning() << "Geçersiz GPS veri paketi.";
            }

            packetLines.clear();
        }
    }

    file.close();
    currentIndex = 0;
    return true;
}

bool GpsDataProcessor::validateNmeaString(const QString& nmeaString) {
    if (!nmeaString.startsWith("$") || !nmeaString.contains('*')) {
        return false;
    }

    int asteriskIndex = nmeaString.indexOf('*');
    QString checksumStr = nmeaString.mid(asteriskIndex + 1);
    bool ok;
    int checksumValue = checksumStr.toInt(&ok, 16);
    if (!ok) return false;

    int calculatedChecksum = 0;
    for (char ch : nmeaString.mid(1, asteriskIndex - 1).toUtf8()) {
        calculatedChecksum ^= ch;
    }

    return calculatedChecksum == checksumValue;
}

bool GpsDataProcessor::parseGpsPacket(const GpsPacket& packet, GpsData& gpsData) {
    QStringList ggaFields = packet.gpgga.split(',');
    if (ggaFields.size() < 6) return false;

    gpsData.latitude = convertToDegrees(ggaFields[2], ggaFields[3]);
    gpsData.longitude = convertToDegrees(ggaFields[4], ggaFields[5]);

    QStringList rmcFields = packet.gprmc.split(',');
    if (rmcFields.size() < 9 || rmcFields[2] != "A") return false;

    gpsData.speed = rmcFields[7].toDouble() * 1.852;
    gpsData.trackHead = rmcFields[8].toDouble();

    return true;
}

double GpsDataProcessor::convertToDegrees(const QString& coordinate, const QString& direction) {
    double rawValue = coordinate.toDouble();
    double degrees = int(rawValue / 100);
    double minutes = rawValue - (degrees * 100);
    double result = degrees + (minutes / 60.0);
    if (direction == "S" || direction == "W") result = -result;
    return result;
}

void GpsDataProcessor::startProcessing() {
    if (!gpsPackets.isEmpty() && !timer->isActive()) {
        timer->start(50); // 20 Hz = 50 ms
    }
}

void GpsDataProcessor::stopProcessing() {
    if (timer->isActive()) {
        timer->stop();
    }
}

void GpsDataProcessor::toggleTracking(bool enable) {
    trackingEnabled = enable;
    qDebug() << "Tracking" << (enable ? "enabled" : "disabled");
}

void GpsDataProcessor::sendUdpPacket(const GpsData& gpsData) {
    QByteArray payload;

    uint16_t header = qToBigEndian<uint16_t>(0xFAFF);
    payload.append(reinterpret_cast<const char*>(&header), sizeof(header));


    float latitudeRad = gpsData.latitude * M_PI / 180.0;
    payload.append(reinterpret_cast<const char*>(&latitudeRad), sizeof(latitudeRad));

    float longitudeRad = gpsData.longitude * M_PI / 180.0;
    payload.append(reinterpret_cast<const char*>(&longitudeRad), sizeof(longitudeRad));


    float speedMs = gpsData.speed / 3.6;
    payload.append(reinterpret_cast<const char*>(&speedMs), sizeof(speedMs));

    float trackHeadRad = gpsData.trackHead * M_PI / 180.0;
    payload.append(reinterpret_cast<const char*>(&trackHeadRad), sizeof(trackHeadRad));

    uint16_t crc16 = calculateCrc16(payload);
    uint16_t crc16BigEndian = qToBigEndian<uint16_t>(crc16);
    payload.append(reinterpret_cast<const char*>(&crc16BigEndian), sizeof(crc16BigEndian));

    udpSocket->writeDatagram(payload, QHostAddress(Constanst::MULTICAST_DESTINATION_ADDRESS), Constanst::MULTICAST_DESTINATION_PORT);
}

uint16_t GpsDataProcessor::calculateCrc16(const QByteArray& data) {
    uint16_t crc = 0xFFFF;
    for (const auto& byte : data) {
        crc ^= static_cast<uint8_t>(byte);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
