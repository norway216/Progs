#include "ImageManager.h"
#include "FrameGenerator.h"
#include <QDateTime>

ImageManager::ImageManager(SonoParameters *params, CineBuffer *cine, UltrasoundImageProvider *provider, QObject *parent)
    : QObject(parent), m_params(params), m_cine(cine), m_provider(provider) {
    m_timer.setInterval(33);
    connect(&m_timer, &QTimer::timeout, this, &ImageManager::onTick);
    connect(m_params, &SonoParameters::parametersCommitted, this, [this]{ if (!m_frozen) onTick(); });
}
QString ImageManager::scanMode() const { return m_scanMode; }
bool ImageManager::frozen() const { return m_frozen; }
int ImageManager::frameNo() const { return m_frameNo; }
double ImageManager::fps() const { return m_fps; }
void ImageManager::start() { m_lastFpsTime = QDateTime::currentMSecsSinceEpoch(); m_timer.start(); emit logMessage("Image pipeline started: FPGA -> Beamformer -> SonoBuffer -> QML viewport"); }
void ImageManager::freeze() { if (m_frozen) return; m_frozen = true; emit frozenChanged(); emit logMessage("ImageManager::freeze(): acquisition paused, last frame kept"); }
void ImageManager::unfreeze() { if (!m_frozen) return; m_frozen = false; emit frozenChanged(); emit logMessage("ImageManager::unfreeze(): realtime acquisition resumed"); }
void ImageManager::switchScanMode(const QString &mode) { if (m_scanMode == mode) return; freeze(); m_scanMode = mode; emit scanModeChanged(); emit logMessage(QString("ImageManager::switchScanMode(%1): updater reused from object pool").arg(mode)); QTimer::singleShot(120, this, &ImageManager::unfreeze); }
void ImageManager::showCineFrame(int index) { QImage img = m_cine->frameAt(index); if (!img.isNull()) { m_provider->setImage(img); emit frameReady(); } }
void ImageManager::onTick() {
    if (m_frozen) return;
    ++m_frameNo;
    QImage img = FrameGenerator::generate(960, 720, m_frameNo, m_scanMode, *m_params, false);
    m_provider->setImage(img);
    m_cine->push(img);
    emit frameReady();
    ++m_fpsCounter;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastFpsTime >= 1000) { m_fps = m_fpsCounter * 1000.0 / double(now - m_lastFpsTime); m_fpsCounter = 0; m_lastFpsTime = now; emit fpsChanged(); }
}
