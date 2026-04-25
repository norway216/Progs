#pragma once
#include <QObject>
#include <QStringList>
#include "SonoParameters.h"
#include "StateManager.h"
#include "CineBuffer.h"
#include "ImageManager.h"
#include "MarkManager.h"
#include "ExamManager.h"
#include "PresetManager.h"
#include "PeripheralManager.h"
#include "UltrasoundImageProvider.h"

class AppController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList logs READ logs NOTIFY logsChanged)
    Q_PROPERTY(SonoParameters* params READ params CONSTANT)
    Q_PROPERTY(StateManager* state READ state CONSTANT)
    Q_PROPERTY(ImageManager* image READ image CONSTANT)
    Q_PROPERTY(CineBuffer* cine READ cine CONSTANT)
    Q_PROPERTY(MarkManager* mark READ mark CONSTANT)
    Q_PROPERTY(ExamManager* exam READ exam CONSTANT)
    Q_PROPERTY(PresetManager* preset READ preset CONSTANT)
    Q_PROPERTY(PeripheralManager* peripheral READ peripheral CONSTANT)
public:
    explicit AppController(UltrasoundImageProvider *provider, QObject *parent = nullptr);
    QStringList logs() const;
    SonoParameters *params();
    StateManager *state();
    ImageManager *image();
    CineBuffer *cine();
    MarkManager *mark();
    ExamManager *exam();
    PresetManager *preset();
    PeripheralManager *peripheral();
    Q_INVOKABLE void boot();
    Q_INVOKABLE void clearLogs();
public slots:
    void addLog(const QString &message);
signals:
    void logsChanged();
private:
    QStringList m_logs;
    SonoParameters m_params;
    StateManager m_state;
    CineBuffer m_cine;
    ImageManager m_image;
    MarkManager m_mark;
    ExamManager m_exam;
    PresetManager m_preset;
    PeripheralManager m_peripheral;
};
