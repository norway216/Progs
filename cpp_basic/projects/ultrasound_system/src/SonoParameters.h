#pragma once
#include <QObject>

class SonoParameters final : public QObject {
    Q_OBJECT
    Q_PROPERTY(int depth READ depth WRITE setDepth NOTIFY depthChanged)
    Q_PROPERTY(int gain READ gain WRITE setGain NOTIFY gainChanged)
    Q_PROPERTY(int dynamicRange READ dynamicRange WRITE setDynamicRange NOTIFY dynamicRangeChanged)
    Q_PROPERTY(int frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(int focus READ focus WRITE setFocus NOTIFY focusChanged)
    Q_PROPERTY(QString preset READ preset WRITE setPreset NOTIFY presetChanged)
public:
    explicit SonoParameters(QObject *parent = nullptr);
    int depth() const;
    int gain() const;
    int dynamicRange() const;
    int frequency() const;
    int focus() const;
    QString preset() const;
public slots:
    void setDepth(int value);
    void setGain(int value);
    void setDynamicRange(int value);
    void setFrequency(int value);
    void setFocus(int value);
    void setPreset(const QString &value);
    void applyAbdomenPreset();
    void applyCardiacPreset();
    void applyVascularPreset();
signals:
    void depthChanged();
    void gainChanged();
    void dynamicRangeChanged();
    void frequencyChanged();
    void focusChanged();
    void presetChanged();
    void parametersCommitted();
private:
    int m_depth = 120;
    int m_gain = 55;
    int m_dynamicRange = 72;
    int m_frequency = 5;
    int m_focus = 60;
    QString m_preset = "Abdomen";
};
