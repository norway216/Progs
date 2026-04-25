#include "SonoParameters.h"

SonoParameters::SonoParameters(QObject *parent) : QObject(parent) {}
int SonoParameters::depth() const { return m_depth; }
int SonoParameters::gain() const { return m_gain; }
int SonoParameters::dynamicRange() const { return m_dynamicRange; }
int SonoParameters::frequency() const { return m_frequency; }
int SonoParameters::focus() const { return m_focus; }
QString SonoParameters::preset() const { return m_preset; }

void SonoParameters::setDepth(int value) { if (m_depth == value) return; m_depth = value; emit depthChanged(); emit parametersCommitted(); }
void SonoParameters::setGain(int value) { if (m_gain == value) return; m_gain = value; emit gainChanged(); emit parametersCommitted(); }
void SonoParameters::setDynamicRange(int value) { if (m_dynamicRange == value) return; m_dynamicRange = value; emit dynamicRangeChanged(); emit parametersCommitted(); }
void SonoParameters::setFrequency(int value) { if (m_frequency == value) return; m_frequency = value; emit frequencyChanged(); emit parametersCommitted(); }
void SonoParameters::setFocus(int value) { if (m_focus == value) return; m_focus = value; emit focusChanged(); emit parametersCommitted(); }
void SonoParameters::setPreset(const QString &value) { if (m_preset == value) return; m_preset = value; emit presetChanged(); emit parametersCommitted(); }

void SonoParameters::applyAbdomenPreset() { setPreset("Abdomen"); setDepth(140); setGain(58); setDynamicRange(78); setFrequency(4); setFocus(70); }
void SonoParameters::applyCardiacPreset() { setPreset("Cardiac"); setDepth(160); setGain(62); setDynamicRange(64); setFrequency(3); setFocus(90); }
void SonoParameters::applyVascularPreset() { setPreset("Vascular"); setDepth(60); setGain(48); setDynamicRange(82); setFrequency(8); setFocus(35); }
