#pragma once
#include <QObject>
#include <QTimer>
#include "SonoParameters.h"
#include "CineBuffer.h"
#include "UltrasoundImageProvider.h"

class ImageManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString scanMode READ scanMode NOTIFY scanModeChanged)
    Q_PROPERTY(bool frozen READ frozen NOTIFY frozenChanged)
    Q_PROPERTY(int frameNo READ frameNo NOTIFY frameReady)
    Q_PROPERTY(double fps READ fps NOTIFY fpsChanged)
public:
    ImageManager(SonoParameters *params, CineBuffer *cine, UltrasoundImageProvider *provider, QObject *parent = nullptr);
    QString scanMode() const;
    bool frozen() const;
    int frameNo() const;
    double fps() const;
public slots:
    void start();
    void freeze();
    void unfreeze();
    void switchScanMode(const QString &mode);
    void showCineFrame(int index);
signals:
    void scanModeChanged();
    void frozenChanged();
    void frameReady();
    void fpsChanged();
    void logMessage(const QString &message);
private slots:
    void onTick();
private:
    SonoParameters *m_params = nullptr;
    CineBuffer *m_cine = nullptr;
    UltrasoundImageProvider *m_provider = nullptr;
    QTimer m_timer;
    QString m_scanMode = "B";
    bool m_frozen = false;
    int m_frameNo = 0;
    int m_fpsCounter = 0;
    double m_fps = 0.0;
    qint64 m_lastFpsTime = 0;
};
