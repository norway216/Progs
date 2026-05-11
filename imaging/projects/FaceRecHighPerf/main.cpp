#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "FaceRecognition.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    FaceRecognition faceRec;
    engine.rootContext()->setContextProperty("faceRec", &faceRec);

    engine.load(QUrl(QStringLiteral("qml/MainPage.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    faceRec.startCamera();

    return app.exec();
}