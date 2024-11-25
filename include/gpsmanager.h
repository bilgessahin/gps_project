#ifndef GPSMANAGER_H
#define GPSMANAGER_H

#include <QObject>

class GpsManager : public QObject
{
    Q_OBJECT
public:
    explicit GpsManager(QObject *parent = nullptr);

signals:
};

#endif // GPSMANAGER_H
