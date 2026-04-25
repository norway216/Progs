#pragma once
#include <QObject>

class StateManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
public:
    explicit StateManager(QObject *parent = nullptr);
    QString state() const;
public slots:
    void postEvent(const QString &eventName);
signals:
    void stateChanged();
    void freezeRequested();
    void unfreezeRequested();
    void cinePlayRequested();
    void cineStopRequested();
    void logMessage(const QString &message);
private:
    void setState(const QString &value);
    QString m_state = "Idle";
};
