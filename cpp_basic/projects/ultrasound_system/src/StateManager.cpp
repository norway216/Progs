#include "StateManager.h"

StateManager::StateManager(QObject *parent) : QObject(parent) {}
QString StateManager::state() const { return m_state; }

void StateManager::postEvent(const QString &eventName) {
    emit logMessage(QString("State event: %1").arg(eventName));
    if (eventName == "probeInserted") setState("RealTimeB");
    else if (eventName == "probeRemoved") setState("Idle");
    else if (eventName == "freeze" && m_state.startsWith("RealTime")) { setState("FreezeB"); emit freezeRequested(); }
    else if (eventName == "unfreeze" && (m_state == "FreezeB" || m_state == "CinePlaying")) { setState("RealTimeB"); emit unfreezeRequested(); }
    else if (eventName == "cinePlay" && m_state == "FreezeB") { setState("CinePlaying"); emit cinePlayRequested(); }
    else if (eventName == "cineStop" && m_state == "CinePlaying") { setState("FreezeB"); emit cineStopRequested(); }
    else if (eventName == "measureStart" && m_state.startsWith("RealTime")) setState("MeasureActive");
    else if (eventName == "measureComplete" && m_state == "MeasureActive") setState("RealTimeB");
    else if (eventName == "saveImage") setState("SavingImage");
}

void StateManager::setState(const QString &value) {
    if (m_state == value) return;
    m_state = value;
    emit stateChanged();
    emit logMessage(QString("State -> %1").arg(m_state));
}
