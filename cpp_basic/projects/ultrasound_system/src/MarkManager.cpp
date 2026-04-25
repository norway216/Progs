#include "MarkManager.h"
#include <QtMath>
#include <QVariantMap>

MarkManager::MarkManager(QObject *parent) : QObject(parent) {}
QString MarkManager::activeTool() const { return m_activeTool; }
QVariantList MarkManager::measurements() const { return m_measurements; }
void MarkManager::activateTool(const QString &tool) { if (m_activeTool == tool) return; m_activeTool = tool; emit activeToolChanged(); emit logMessage(QString("MarkManager::activateTool(%1)").arg(tool)); }
void MarkManager::addDistanceMeasurement(double x1, double y1, double x2, double y2) { QVariantMap m; double pixels = std::hypot(x2 - x1, y2 - y1); m["type"] = "Distance"; m["value"] = QString::number(pixels * 0.12, 'f', 1) + " mm"; m_measurements.prepend(m); emit measurementsChanged(); emit logMessage("Measurement result saved to report context"); }
void MarkManager::addComment(const QString &text) { QVariantMap m; m["type"] = "Comment"; m["value"] = text; m_measurements.prepend(m); emit measurementsChanged(); emit logMessage("Comment overlay added"); }
void MarkManager::clear() { m_measurements.clear(); emit measurementsChanged(); }
