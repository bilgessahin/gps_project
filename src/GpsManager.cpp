#include <include/GpsManager.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <cmath>


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
    payload.append(reinterpret_cast<const char*>(&gpsData.latitude), sizeof(gpsData.latitude));
    payload.append(reinterpret_cast<const char*>(&gpsData.longitude), sizeof(gpsData.longitude));
    payload.append(reinterpret_cast<const char*>(&gpsData.speed), sizeof(gpsData.speed));
    payload.append(reinterpret_cast<const char*>(&gpsData.trackHead), sizeof(gpsData.trackHead));

    udpSocket->writeDatagram(payload, QHostAddress("233.10.10.10"), 57003);
}


// GpsDataProcessor::GpsDataProcessor(QObject* parent)
//     : QObject(parent), timer(new QTimer(this)), currentIndex(0) {
//     connect(timer, &QTimer::timeout, this, [this]() {
//         if (currentIndex < gpsPackets.size()) {
//             GpsData gpsData;
//             if (parseGpsPacket(gpsPackets[currentIndex], gpsData)) {
//                 emit gpsDataUpdated(gpsData.latitude, gpsData.longitude, gpsData.speed, gpsData.trackHead);
//             }
//             currentIndex++;
//         } else {
//             timer->stop();
//             qDebug() << "Tüm GPS verileri işlendi.";
//         }
//     });
// }

// bool GpsDataProcessor::loadFile(const QString& filePath) {
//     QFile file(filePath);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qWarning() << "Dosya açılamadı:" << file.errorString();
//         return false;
//     }

//     QTextStream in(&file);
//     gpsPackets.clear();

//     QStringList packetLines;

//     while (!in.atEnd()) {
//         QString line = in.readLine().trimmed();
//         if (line.isEmpty()) continue;

//         packetLines.append(line);

//         // Her üç satır bir GPS paketidir
//         if (packetLines.size() == 3) {
//             // 1. Adım: Sıralama Kontrolü
//             if (packetLines[0].startsWith("$GPGGA") &&
//                 packetLines[1].startsWith("$GPGSA") &&
//                 packetLines[2].startsWith("$GPRMC")) {

//                 // 2. Adım: Checksum Kontrolü
//                 bool valid = true;
//                 for (const QString& packetLine : packetLines) {
//                     if (!validateNmeaString(packetLine)) {
//                         qWarning() << "Checksum doğrulaması başarısız. Geçersiz paket satırı:" << packetLine;
//                         valid = false;
//                         break;
//                     }
//                 }

//                 if (valid) {
//                     GpsPacket packet = { packetLines[0], packetLines[1], packetLines[2] };
//                     gpsPackets.append(packet);
//                 } else {
//                     qWarning() << "Checksum hatası nedeniyle paket geçersiz:" << packetLines;
//                 }

//             } else {
//                 qWarning() << "Geçersiz GPS veri paketi. Sıralama hatası:" << packetLines;
//             }

//             // İşlenen satırları temizle
//             packetLines.clear();
//         }
//     }

//     file.close();
//     currentIndex = 0;
//     qDebug() << "Toplam geçerli paket sayısı:" << gpsPackets.size();
//     return true;
// }

// bool GpsDataProcessor::validateNmeaString(const QString& nmeaString) {
//     if (!nmeaString.startsWith("$") || !nmeaString.contains('*')) {
//         qWarning() << "NMEA String formatı geçersiz: " << nmeaString;
//         return false;
//     }

//     // '*' karakterinin yerini bul
//     int asteriskIndex = nmeaString.indexOf('*');
//     QString checksumStr = nmeaString.mid(asteriskIndex + 1).trimmed(); // Fazladan boşlukları temizle
//     bool ok;
//     int checksumValue = checksumStr.toInt(&ok, 16);
//     if (!ok) {
//         qWarning() << "Checksum değeri geçersiz: " << checksumStr;
//         return false;
//     }

//     // XOR ile checksum hesaplama
//     int calculatedChecksum = 0;
//     QString checksumData = nmeaString.mid(1, asteriskIndex - 1); // '$' sonrası ve '*' öncesi
//     for (char ch : checksumData.toUtf8()) {
//         calculatedChecksum ^= ch;
//     }

//     // Debug bilgileri
//     // qDebug() << "Raw NMEA String:" << nmeaString.toUtf8(); // Orijinal ham string
//     // qDebug() << "Data to be checksummed:" << nmeaString.mid(1, asteriskIndex - 1); // Hesaplamada kullanılan veri
//     // qDebug() << "Checksum Data:" << checksumData; // XOR işlemine dahil edilen karakterler
//     // qDebug() << "calculatedChecksum (Hex):" << QString::number(calculatedChecksum, 16).toUpper();
//     // qDebug() << "checksumValue (Hex):" << QString::number(checksumValue, 16).toUpper();

//     // Hesaplanan ve beklenen checksum değerlerini karşılaştır
//     if (calculatedChecksum != checksumValue) {
//         qWarning() << "Checksum doğrulaması başarısız:"
//                    << "Hesaplanan:" << QString::number(calculatedChecksum, 16).toUpper()
//                    << "Beklenen:" << QString::number(checksumValue, 16).toUpper();
//     }

//     return (calculatedChecksum == checksumValue);
// }



// bool GpsDataProcessor::parseGpsPacket(const GpsPacket& packet, GpsData& gpsData) {
//     // GPGGA Satırı (Latitude, Longitude)
//     QStringList ggaFields = packet.gpgga.split(',');
//     if (ggaFields.size() < 6) { // Yeterli alan kontrolü
//         qWarning() << "GPGGA verisi geçersiz.";
//         return false;
//     }

//     qDebug() << "ggaFields " << ggaFields;
//     // Latitude (DDmm.mmm → DD.dddd)
//     double rawLatitude = ggaFields[2].toDouble();
//     double latDegrees = int(rawLatitude / 100);
//     double latMinutes = rawLatitude - (latDegrees * 100);
//     gpsData.latitude = latDegrees + (latMinutes / 60.0);

//     qDebug()<< "latitude" << gpsData.latitude;

//     if (ggaFields[3] == "S") gpsData.latitude = -gpsData.latitude;

//     // Longitude (DDDmm.mmm → DDD.dddd)
//     double rawLongitude = ggaFields[4].toDouble();
//     double lonDegrees = int(rawLongitude / 100);
//     double lonMinutes = rawLongitude - (lonDegrees * 100);
//     gpsData.longitude = lonDegrees + (lonMinutes / 60.0);
//     if (ggaFields[5] == "W") gpsData.longitude = -gpsData.longitude;

//     // GPRMC Satırı (Speed, Track Head)
//     QStringList rmcFields = packet.gprmc.split(',');
//     if (rmcFields.size() < 9 || rmcFields[2] != "A") { // A: Valid data
//         qWarning() << "GPRMC verisi geçersiz.";
//         return false;
//     }

//     qDebug() << "rmcFields " << rmcFields;
//     // Speed (Knots → km/h)
//     gpsData.speed = rmcFields[7].toDouble() * 1.852;

//     // Track Head
//     gpsData.trackHead = rmcFields[8].toDouble();

//     return true;
// }


// void GpsDataProcessor::startProcessing() {
//     if (!gpsPackets.isEmpty() && !timer->isActive()) {
//         timer->start(30);
//         qDebug() << "GPS veri işleme başlatıldı.";
//     }
// }

// void GpsDataProcessor::stopProcessing() {
//     if (timer->isActive()) {
//         timer->stop();
//         qDebug() << "GPS veri işleme durduruldu.";
//     }
// }
