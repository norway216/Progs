#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "AppController.h"
#include "UltrasoundImageProvider.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    QQmlApplicationEngine engine;
    auto *provider = new UltrasoundImageProvider();
    engine.addImageProvider("ultrasound", provider);
    AppController controller(provider);
    engine.rootContext()->setContextProperty("app", &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty()) return -1;
    controller.boot();
    return app.exec();
}
