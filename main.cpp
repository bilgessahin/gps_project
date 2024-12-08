#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <include/GpsManager.h>


int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    GpsDataProcessor gpsProcessor;

    engine.rootContext()->setContextProperty("gpsProcessor", &gpsProcessor);

    const QUrl url(QStringLiteral("qrc:/qrc/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    const QString filePath = "/home/bilge/Desktop/GpsApp/gps_nmea_out.txt";
    if (!gpsProcessor.loadFile(filePath)) {
        qWarning() << "GPS dosyası yüklenemedi. Program kapatılıyor.";
        return -1;
    }

    return app.exec();
}

