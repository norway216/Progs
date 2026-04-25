#include "UltrasoundImageProvider.h"
#include <QMutexLocker>

UltrasoundImageProvider::UltrasoundImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}
QImage UltrasoundImageProvider::requestImage(const QString &, QSize *size, const QSize &requestedSize) {
    QMutexLocker locker(&m_mutex);
    QImage result = m_image.isNull() ? QImage(960, 720, QImage::Format_RGB32) : m_image;
    if (result.isNull()) result.fill(Qt::black);
    if (size) *size = result.size();
    if (requestedSize.isValid()) result = result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return result;
}
void UltrasoundImageProvider::setImage(const QImage &image) { QMutexLocker locker(&m_mutex); m_image = image; }
