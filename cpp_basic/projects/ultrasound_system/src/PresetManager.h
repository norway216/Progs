#pragma once
#include <QObject>
#include "SonoParameters.h"

class PresetManager final : public QObject {
    Q_OBJECT
public:
    explicit PresetManager(SonoParameters *params, QObject *parent = nullptr);
public slots:
    void switchPreset(const QString &name);
signals:
    void logMessage(const QString &message);
private:
    SonoParameters *m_params = nullptr;
};
