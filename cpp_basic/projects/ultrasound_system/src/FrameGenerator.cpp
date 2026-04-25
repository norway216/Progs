#include "FrameGenerator.h"
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>

QImage FrameGenerator::generate(int width, int height, int frameNo, const QString &mode, const SonoParameters &params, bool frozen) {
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(QColor(5, 10, 16));
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QPoint center(width / 2, height - 20);
    const double maxRadius = height * 0.95;
    const double gain = params.gain() / 100.0;
    const double phase = frozen ? 0.0 : frameNo * 0.08;
    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            double dx = x - center.x();
            double dy = center.y() - y;
            double r = std::sqrt(dx * dx + dy * dy);
            double a = std::atan2(dx, dy);
            bool sector = std::abs(a) < 0.62 && r < maxRadius && r > 16;
            int v = 8;
            if (sector) {
                double tissue = 0.5 + 0.5 * std::sin(r * 0.065 + std::sin(a * 8.0 + phase) * 1.8);
                double speckle = QRandomGenerator::global()->bounded(42) / 255.0;
                double vessel = std::exp(-std::pow((dx - std::sin(phase) * 20.0) / 48.0, 2.0) - std::pow((dy - height * 0.45) / 24.0, 2.0));
                v = qBound(0, int((35 + 155 * tissue + 60 * speckle - 75 * vessel) * (0.45 + gain)), 255);
            }
            line[x] = qRgb(v, v, v);
        }
    }
    p.setPen(QPen(QColor(70, 190, 255, 160), 1));
    for (int i = 1; i <= 6; ++i) {
        int r = int(maxRadius * i / 7.0);
        p.drawArc(QRect(center.x() - r, center.y() - r, 2 * r, 2 * r), 54 * 16, 72 * 16);
    }
    p.drawLine(center, QPoint(center.x() - int(maxRadius * 0.58), center.y() - int(maxRadius * 0.82)));
    p.drawLine(center, QPoint(center.x() + int(maxRadius * 0.58), center.y() - int(maxRadius * 0.82)));
    if (mode.contains("Color") || mode.contains("Triplex")) {
        for (int i = 0; i < 50; ++i) {
            int rx = width / 2 - 90 + QRandomGenerator::global()->bounded(180);
            int ry = height / 2 - 65 + QRandomGenerator::global()->bounded(120);
            QColor c = (i % 2 == 0) ? QColor(245, 60, 40, 150) : QColor(20, 80, 240, 150);
            p.setBrush(c); p.setPen(Qt::NoPen); p.drawEllipse(QPoint(rx, ry), 3 + i % 4, 3 + i % 4);
        }
    }
    if (mode.contains("M")) {
        p.fillRect(QRect(20, height - 115, width - 40, 88), QColor(10, 16, 24, 210));
        p.setPen(QPen(QColor(200, 240, 255, 190), 1));
        for (int x = 24; x < width - 24; x += 3) {
            int y = height - 70 + int(22 * std::sin(x * 0.05 + phase * 5));
            p.drawPoint(x, y);
        }
    }
    p.setPen(QPen(QColor(255, 210, 50), 2));
    p.drawLine(width / 2 - 45, height / 2 - 10, width / 2 + 45, height / 2 + 18);
    p.setPen(QColor(255, 240, 140));
    p.drawText(width / 2 - 28, height / 2 - 20, "42.3 mm");
    p.setPen(QColor(210, 235, 255));
    p.drawText(20, 28, QString("%1  D:%2mm G:%3 DR:%4 F:%5MHz").arg(mode).arg(params.depth()).arg(params.gain()).arg(params.dynamicRange()).arg(params.frequency()));
    if (frozen) {
        p.setPen(QPen(QColor(255, 220, 40), 2));
        p.drawText(width - 95, 30, "FREEZE");
    }
    return image;
}
