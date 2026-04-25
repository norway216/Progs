#pragma once
#include <QObject>
#include <QVariantList>

class MarkManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeTool READ activeTool NOTIFY activeToolChanged)
    Q_PROPERTY(QVariantList measurements READ measurements NOTIFY measurementsChanged)
public:
    explicit MarkManager(QObject *parent = nullptr);
    QString activeTool() const;
    QVariantList measurements() const;
public slots:
    void activateTool(const QString &tool);
    void addDistanceMeasurement(double x1, double y1, double x2, double y2);
    void addComment(const QString &text);
    void clear();
signals:
    void activeToolChanged();
    void measurementsChanged();
    void logMessage(const QString &message);
private:
    QString m_activeTool = "None";
    QVariantList m_measurements;
};
