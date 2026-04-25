#pragma once
#include <QObject>
#include <QImage>
#include <QVector>

class CineBuffer final : public QObject {
    Q_OBJECT
    Q_PROPERTY(int size READ size NOTIFY changed)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
public:
    explicit CineBuffer(QObject *parent = nullptr);
    void push(const QImage &image);
    QImage frameAt(int index) const;
    int size() const;
    int currentIndex() const;
public slots:
    void clear();
    void setCurrentIndex(int index);
signals:
    void changed();
    void currentIndexChanged();
private:
    QVector<QImage> m_frames;
    int m_currentIndex = 0;
    int m_capacity = 180;
};
