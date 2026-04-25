#pragma once
#include <QImage>
#include <QString>
#include "SonoParameters.h"

class FrameGenerator final {
public:
    static QImage generate(int width, int height, int frameNo, const QString &mode, const SonoParameters &params, bool frozen);
};
