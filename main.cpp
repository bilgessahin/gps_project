// // #include <QGuiApplication>
// // #include <QQmlApplicationEngine>

// // #include <include/GpsManager.h>

// // int main(int argc, char *argv[])
// // {
// //     qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

// //     QGuiApplication app(argc, argv);

// //     GpsManager manager;
// //     if (manager.loadFile("/home/bilge/Desktop/GpsApp/gps_nmea_out.txt")) {
// //         manager.parseGpsData();
// //     }


// //     // QQmlApplicationEngine engine;
// //     // const QUrl url(QStringLiteral("qrc:/qrc/qml/main.qml"));
// //     // QObject::connect(
// //     //     &engine,
// //     //     &QQmlApplicationEngine::objectCreated,
// //     //     &app,
// //     //     [url](QObject *obj, const QUrl &objUrl) {
// //     //         if (!obj && url == objUrl)
// //     //             QCoreApplication::exit(-1);
// //     //     },
// //     //     Qt::QueuedConnection);
// //     // engine.load(url);

// //     return app.exec();
// // }


// #include <QFile>
// #include <QTextStream>
// #include <QString>
// #include <QStringList>
// #include <QDebug>

// // NMEA Verisi Geçerlilik Kontrolü (Checksum hesaplama fonksiyonu)
// bool validateNmeaString(const QString& nmeaString) {
//     if (!nmeaString.startsWith("$") || !nmeaString.contains('*')) {
//         return false;
//     }

//     int asteriskIndex = nmeaString.indexOf('*');
//     QString checksumStr = nmeaString.mid(asteriskIndex + 1);
//     qDebug() << "checksmstr " << checksumStr;

//     bool ok;
//     int checksumValue = checksumStr.toInt(&ok, 16);
//     if (!ok) {
//         return false;
//     }

//     // Checksum hesaplama
//     int calculatedChecksum = 0;
//     for (int i = 1; i < asteriskIndex; ++i) { // '$' karakteri dahil edilmez
//         calculatedChecksum ^= nmeaString.at(i).toLatin1();
//     }

//     qDebug() << "calculatedChecksum = " << calculatedChecksum;
//     qDebug() << "checksumValue = " << checksumValue;

//     return (calculatedChecksum == checksumValue);
// }

// // GPGGA, GPGSA, GPRMC verisini ayrıştırma ve kontrol
// void parseNmeaFile(const QString& filePath) {
//     QFile file(filePath);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qDebug() << "Dosya açılamadı:" << file.errorString();
//         return;
//     }

//     QTextStream in(&file);
//     QStringList validPackets; // Geçerli GPS veri paketlerini saklama

//     while (!in.atEnd()) {
//         QString line = in.readLine().trimmed();

//         // Boş satırları atla
//         if (line.isEmpty()) {
//             continue;
//         }

//         // Geçerlilik kontrolü
//         if (validateNmeaString(line)) {
//             validPackets.append(line);
//         } else {
//             qDebug() << "Geçersiz NMEA String:" << line;
//         }
//     }

//     file.close();

//     // Geçerli NMEA verilerini ekrana yazdırma
//     for (const QString& packet : validPackets) {
//         qDebug() << "Geçerli NMEA String:" << packet;
//     }
// }

// int main(int argc, char* argv[]) {
//     QString filePath = "/home/bilge/Desktop/GpsApp/gps_nmea_out.txt"; // Girdi dosyası yolu
//     //parseNmeaFile(filePath);
//     validateNmeaString("$GPRMC,081917.016,A,4000.396,N,03240.855,E,363.0,217.5,191124,000.0,W*77");
//     return 0;
// }
// #include <QCoreApplication>
// #include "include/GpsManager.h"

// int main(int argc, char* argv[]) {
//     QCoreApplication a(argc, argv);

//     GpsDataProcessor processor;
//     if (processor.loadFile("/home/bilge/Desktop/GpsApp/gps_nmea_out.txt")) {
//         QList<GpsData> dataList = processor.getProcessedData();
//         for (const GpsData& data : dataList) {
//             qDebug() << "Latitude:" << data.latitude
//                      << "Longitude:" << data.longitude
//                      << "Speed:" << data.speed
//                      << "Track Head:" << data.trackHead;
//         }
//     } else {
//         qDebug() << "Dosya yüklenemedi.";
//     }

//     return a.exec();
// }

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <include/GpsManager.h>

// int main(int argc, char* argv[]) {
//     QGuiApplication app(argc, argv);

//     QQmlApplicationEngine engine;
//     GpsDataProcessor gpsProcessor;

//     // GpsDataProcessor'ı QML'e bağlama
//     engine.rootContext()->setContextProperty("gpsProcessor", &gpsProcessor);

//     const QUrl url(QStringLiteral("qrc:/qrc/qml/main.qml"));
//     QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
//                      &app, [url](QObject* obj, const QUrl& objUrl) {
//                          if (!obj && url == objUrl)
//                              QCoreApplication::exit(-1);
//                      }, Qt::QueuedConnection);
//     engine.load(url);

//     return app.exec();
// }



int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // GPS Data Processor nesnesi
    GpsDataProcessor gpsProcessor;

    // QML'e GPS Processor'ı aktarıyoruz
    engine.rootContext()->setContextProperty("gpsProcessor", &gpsProcessor);

    const QUrl url(QStringLiteral("qrc:/qrc/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    // Program başladığında dosya otomatik olarak yüklenir
    const QString filePath = "/home/bilge/Desktop/GpsApp/gps_nmea_out.txt";
    if (!gpsProcessor.loadFile(filePath)) {
        qWarning() << "GPS dosyası yüklenemedi. Program kapatılıyor.";
        return -1;
    }

    return app.exec();
}

