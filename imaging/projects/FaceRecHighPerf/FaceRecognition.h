#pragma once
#include <QObject>
#include <QString>
#include <QTimer>
#include <opencv2/opencv.hpp>

struct FaceInfo {
    QString name;
    int id;
    QString result;
};

class FaceRecognition : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY infoChanged)
    Q_PROPERTY(int id READ id NOTIFY infoChanged)
    Q_PROPERTY(QString result READ result NOTIFY infoChanged)
public:
    explicit FaceRecognition(QObject* parent = nullptr);
    QString name() const { return currentFace.name; }
    int id() const { return currentFace.id; }
    QString result() const { return currentFace.result; }

    Q_INVOKABLE void startCamera();
signals:
    void frameReady(const QImage& img);
    void infoChanged();

private slots:
    void processFrame();
private:
    cv::VideoCapture cap;
    QTimer timer;
    FaceInfo currentFace;
};