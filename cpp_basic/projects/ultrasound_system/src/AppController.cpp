#include "AppController.h"
#include <QDateTime>

AppController::AppController(UltrasoundImageProvider *provider, QObject *parent)
    : QObject(parent), m_image(&m_params, &m_cine, provider, this), m_preset(&m_params, this) {
    auto wireLog = [this](QObject *obj) { connect(obj, SIGNAL(logMessage(QString)), this, SLOT(addLog(QString))); };
    wireLog(&m_state); wireLog(&m_image); wireLog(&m_mark); wireLog(&m_exam); wireLog(&m_preset); wireLog(&m_peripheral);
    connect(&m_peripheral, &PeripheralManager::probeInserted, &m_state, [this]{ m_state.postEvent("probeInserted"); });
    connect(&m_peripheral, &PeripheralManager::probeRemoved, &m_state, [this]{ m_state.postEvent("probeRemoved"); });
    connect(&m_state, &StateManager::freezeRequested, &m_image, &ImageManager::freeze);
    connect(&m_state, &StateManager::unfreezeRequested, &m_image, &ImageManager::unfreeze);
    connect(&m_state, &StateManager::cinePlayRequested, this, [this]{ addLog("SonoBuffers::loadCine(): playback ready"); });
    connect(&m_state, &StateManager::cineStopRequested, this, [this]{ addLog("SonoBuffers: cine playback stopped"); });
    connect(&m_cine, &CineBuffer::currentIndexChanged, &m_image, [this]{ if (m_state.state() == "CinePlaying") m_image.showCineFrame(m_cine.currentIndex()); });
}
QStringList AppController::logs() const { return m_logs; }
SonoParameters *AppController::params() { return &m_params; }
StateManager *AppController::state() { return &m_state; }
ImageManager *AppController::image() { return &m_image; }
CineBuffer *AppController::cine() { return &m_cine; }
MarkManager *AppController::mark() { return &m_mark; }
ExamManager *AppController::exam() { return &m_exam; }
PresetManager *AppController::preset() { return &m_preset; }
PeripheralManager *AppController::peripheral() { return &m_peripheral; }
void AppController::boot() { addLog("main()->MainModule: managers initialized, SCXML-like state machine loaded"); m_image.start(); }
void AppController::clearLogs() { m_logs.clear(); emit logsChanged(); }
void AppController::addLog(const QString &message) { m_logs.prepend(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + message); while (m_logs.size() > 80) m_logs.removeLast(); emit logsChanged(); }
