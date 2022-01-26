#pragma once

#include <QString>

struct CalibrationFactory {
    static bool makeRGB(const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);
    static bool makeACT(const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);
    static bool makeABar(const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);
};
