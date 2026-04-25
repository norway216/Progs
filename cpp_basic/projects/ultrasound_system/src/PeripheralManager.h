#pragma once
#include <QObject>

class PeripheralManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool probeConnected READ probeConnected NOTIFY probeChanged)
    Q_PROPERTY(QString probeName READ probeName NOTIFY probeChanged)
public:
    explicit PeripheralManager(QObject *parent = nullptr);
    bool probeConnected() const;
    QString probeName() const;
public slots:
    void toggleProbe();
    void printImage();
    void saveToUsb();
signals:
    void probeChanged();
    void probeInserted();
    void probeRemoved();
    void logMessage(const QString &message);
private:
    bool m_probeConnected = false;
    QString m_probeName = "None";
};
