#include "FaceRecognition.h"
#include <QImage>

FaceRecognition::FaceRecognition(QObject* parent)
    : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &FaceRecognition::processFrame);
}

void FaceRecognition::startCamera() {
    cap.open(0);
    if(cap.isOpened()) {
        timer.start(30); // 30ms一帧 ~ 33FPS
    }
}

void FaceRecognition::processFrame() {
    cv::Mat frame;
    cap >> frame;
    if(frame.empty()) return;

    // 模拟检测: 画人脸框
    cv::rectangle(frame, cv::Point(100,100), cv::Point(300,300), cv::Scalar(0,255,0), 2);

    // 模拟识别信息
    currentFace.name = "张三";
    currentFace.id = 1001;
    currentFace.result = "识别成功";
    emit infoChanged();

    // 转QImage发送给QML
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage img((uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    emit frameReady(img.copy());
}