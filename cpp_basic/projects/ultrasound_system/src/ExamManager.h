#pragma once
#include <QObject>
#include <QDateTime>

class ExamManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString patientName READ patientName NOTIFY patientChanged)
    Q_PROPERTY(QString patientId READ patientId NOTIFY patientChanged)
    Q_PROPERTY(QString lastArchivePath READ lastArchivePath NOTIFY archiveChanged)
public:
    explicit ExamManager(QObject *parent = nullptr);
    QString patientName() const;
    QString patientId() const;
    QString lastArchivePath() const;
public slots:
    void registerPatient(const QString &name, const QString &id);
    void saveCurrentImage();
    void sendToPacs();
signals:
    void patientChanged();
    void archiveChanged();
    void logMessage(const QString &message);
private:
    QString m_patientName = "Anonymous";
    QString m_patientId = "P0001";
    QString m_lastArchivePath;
};
