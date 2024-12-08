#ifndef GPSDATAPROCESSOR_H
#define GPSDATAPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QList>
#include <QUdpSocket>

struct GpsData {
    double latitude;
    double longitude;
    double speed;
    double trackHead;
};

struct GpsPacket {
    QString gpgga;
    QString gpgsa;
    QString gprmc;
};

class GpsDataProcessor : public QObject {
    Q_OBJECT

public:
    explicit GpsDataProcessor(QObject* parent = nullptr);
    Q_INVOKABLE bool loadFile(const QString& filePath);
    Q_INVOKABLE void startProcessing();
    Q_INVOKABLE void stopProcessing();
    Q_INVOKABLE void toggleTracking(bool enable);

signals:
    void gpsDataUpdated(double latitude, double longitude, double speed, double trackHead);

private:
    QList<GpsPacket> gpsPackets;
    QTimer* timer;
    QUdpSocket* udpSocket;
    int currentIndex;
    bool trackingEnabled;

    bool validateNmeaString(const QString& nmeaString);
    bool parseGpsPacket(const GpsPacket& packet, GpsData& gpsData);
    double convertToDegrees(const QString& coordinate, const QString& direction);
    void sendUdpPacket(const GpsData& gpsData);
    uint16_t calculateCrc16(const QByteArray& data);
};

#endif // GPSDATAPROCESSOR_H
