#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class UltrasoundImageProvider final : public QQuickImageProvider {
public:
    UltrasoundImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void setImage(const QImage &image);
private:
    QImage m_image;
    QMutex m_mutex;
};
