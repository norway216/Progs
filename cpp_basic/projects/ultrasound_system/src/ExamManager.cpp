#include "ExamManager.h"

ExamManager::ExamManager(QObject *parent) : QObject(parent) {}
QString ExamManager::patientName() const { return m_patientName; }
QString ExamManager::patientId() const { return m_patientId; }
QString ExamManager::lastArchivePath() const { return m_lastArchivePath; }
void ExamManager::registerPatient(const QString &name, const QString &id) { m_patientName = name.trimmed().isEmpty() ? "Anonymous" : name.trimmed(); m_patientId = id.trimmed().isEmpty() ? "P0001" : id.trimmed(); emit patientChanged(); emit logMessage(QString("ExamManager::registerPatient(): %1 / %2").arg(m_patientName, m_patientId)); }
void ExamManager::saveCurrentImage() { m_lastArchivePath = QString("archive/%1_%2.dcm").arg(m_patientId, QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")); emit archiveChanged(); emit logMessage(QString("ExamManager::saveCurrentImage(): DICOM object prepared -> %1").arg(m_lastArchivePath)); }
void ExamManager::sendToPacs() { emit logMessage("DicomManager::sendToPACS(): simulated C-STORE success"); }
