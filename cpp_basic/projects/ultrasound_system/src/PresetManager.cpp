#include "PresetManager.h"

PresetManager::PresetManager(SonoParameters *params, QObject *parent) : QObject(parent), m_params(params) {}
void PresetManager::switchPreset(const QString &name) { emit logMessage(QString("PresetManager::switchPreset(%1): batch SonoParameters update").arg(name)); if (name == "Cardiac") m_params->applyCardiacPreset(); else if (name == "Vascular") m_params->applyVascularPreset(); else m_params->applyAbdomenPreset(); }
