#pragma once

#include <vector>

#include <QImage>
#include <QString>

struct CalibrationFactory
{
    enum PatternType
    {
        RGB,
        ACT,
        ALIGN_BAR
    };

    static bool makeRGB(const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);
    static bool makeACT(const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);
    static bool makeABar(const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);
    static bool makePattern(PatternType type, const QString& filePath, int imageWidth, int imageHeight, int rows, int columns);

    /**
     * @brief getPattern - provides 'number' images of size 'width' x 'height'
     *                     respective to provided pattern type
     * @param type - pattern type
     * @param width - width of image
     * @param height - height of image
     * @param number - number of images to be provided
     * @return list of calibration images
     */
    static std::vector<QImage> getPattern(PatternType type, int width, int height, int number);
};
