#include "CalibrationFactory.h"

#include <vector>

#include <QImage>
#include <QPainter>
#include <QFontMetrics>
#include <QLinearGradient>

namespace {

const std::vector<QColor> RGB_SET = {
    QColor{Qt::red},
    QColor{Qt::green},
    QColor{Qt::blue},
    QColor{Qt::yellow},
    QColor{Qt::magenta},
    QColor{Qt::gray},
    QColor{Qt::cyan}
};

const std::vector<QColor> ACT_SET = {
    QColor{0, 64, 0},
    QColor{0, 127, 0},
//    QColor{0, 191, 0}
};

std::vector<QColor> getColorsSet(const std::vector<QColor>& predefined, size_t size)
{
    std::vector<QColor> colorSet;
    colorSet.reserve(size);

    while (colorSet.size() < size) {
        colorSet.insert(colorSet.end(), predefined.begin(), predefined.begin() + std::min(predefined.size(), size - colorSet.size()));
    }
    return colorSet;
}

void drawMatrix(QPainter& painter, const QRectF& boundRect, size_t matrixSize, const QString& text)
{
    const qreal rows = matrixSize;
    const qreal columns = matrixSize;
    const qreal textPixelSize = 90;
    qreal xOffset = 0;
    qreal yOffset = 0;

    painter.setBrush(QBrush{Qt::black});
    QFont f{"Arrial"};
    f.setPixelSize(textPixelSize);
    painter.setFont(f);

    QFontMetrics fm = painter.fontMetrics();
    const qreal textSize = std::max(fm.horizontalAdvance(text), fm.height());

    for (size_t i = 0; i < rows; ++i) {
        yOffset = i * boundRect.height() / (rows - 1) + boundRect.y();

        for (size_t j = 0; j < columns; ++j) {
            xOffset = j * boundRect.width() / (columns - 1) + boundRect.x();

            QRectF textRect {
                            qBound(boundRect.left(), xOffset - textSize / 2, boundRect.right() - textSize),
                            qBound(boundRect.top(), yOffset - textSize / 2, boundRect.bottom() - textSize),
                            textSize, textSize};

            painter.setBrush(QBrush{Qt::black});
            int allign = Qt::AlignCenter;
            if (j == 0) {
                allign = Qt::AlignLeft | Qt::AlignVCenter;
            } else if (j == columns - 1) {
                allign = Qt::AlignRight | Qt::AlignVCenter;
            }
            painter.drawText(textRect, allign, text);
        }
    }
}

bool makeRGBImpl(QImage& image, int rows, int columns)
{
    QPainter painter;
    if (image.width() <= 0 || image.height() <= 0 || rows <= 0 || columns <= 0 ||
        !painter.begin(&image)) {
        return false;
    }

    const qreal imageWidth = image.width();
    const qreal imageHeight = image.height();
    qreal xOffset = 0;

    // draw colored columns
    const auto columnColors = getColorsSet(RGB_SET, columns);
    for (size_t i = 0; i < columnColors.size(); ++i) {
        qreal width = i == columnColors.size() - 1 ? image.width() - xOffset : imageWidth / columnColors.size();
        painter.setBrush(QBrush{columnColors[i]});
        painter.drawRect(QRectF{xOffset, 0, width, imageHeight});
        xOffset += width;
    }

    const auto margin = 20;
    const qreal cellWidth = imageWidth / columns;
    const qreal cellHeight = imageHeight / rows;
    const auto innerMatrixSize = 3;
    int value = 0;

    xOffset = 0;
    qreal yOffset = 0;

    for (int i = 0; i < rows; ++i) {
        yOffset = i * cellHeight + margin;

        for (int j = 0; j < columns; ++j) {
            xOffset = j * cellWidth + margin;

            drawMatrix(painter,
                       {xOffset, yOffset, cellWidth - margin * 2 , cellHeight - margin * 2},
                       innerMatrixSize, QString::number(value++));
        }
    }

    return true;
}

//////////////// ACT
bool drawACTRows(QPainter& painter, qreal imageWidth, qreal imageHeight, int rows)
{
    if (rows <= 0) {
        return false;
    }

    const auto coloredRowsNumber = ACT_SET.size() * rows;
    const auto colorsSet = getColorsSet(ACT_SET, coloredRowsNumber);
    const qreal height = imageHeight / coloredRowsNumber;
    qreal yOffset = 0;
    for (const auto& color : colorsSet) {
        painter.setBrush(QBrush{color});
        painter.drawRect(QRectF{0, yOffset, imageWidth, height});
        yOffset += height;
    }
    return true;
}

bool drawACTColumns(QPainter& painter, qreal imageWidth, qreal imageHeight, int rows, int columns)
{
    if (rows <= 0 || columns <= 0) {
        return false;
    }

    const qreal pinWidth = 20;
    const qreal pinWidthHalf = pinWidth / 2;
    const qreal pinHeightRatio = 0.9;
    const int stripesNumber = rows * ACT_SET.size();
    const qreal stripeHeight = imageHeight / stripesNumber;
    const qreal pinHeight = stripeHeight * pinHeightRatio;
    const qreal pinYOffset = stripeHeight * (1 - pinHeightRatio) / 2;
    const qreal hOffset = qMax(qMin(75.0, imageWidth / columns / 4), 1.5 * pinWidth);
    const qreal startX = imageHeight / columns / 2 - hOffset;

    qreal yOffset = 0;
    qreal xOffset = 0;
    for (int i = 0; i < stripesNumber; ++i) {
        yOffset = i * stripeHeight + pinYOffset;
        xOffset = startX + i / ACT_SET.size() * hOffset;
        for (int j = 0; j < columns; ++j) {
            if (xOffset < pinWidthHalf) {
                xOffset += imageWidth;
            }

            if (xOffset > imageWidth) {
                xOffset -= imageWidth;
            }

            xOffset = qBound(pinWidthHalf, xOffset, imageWidth - pinWidth);
            painter.setBrush(QBrush{QColor{0, 255, 0}});
            QRectF pinRect{xOffset - pinWidthHalf, yOffset, pinWidth, pinHeight};
            painter.drawRect(pinRect);

            QLinearGradient gradient {pinRect.topLeft(), pinRect.bottomRight()};
            gradient.setColorAt(0, Qt::transparent);
            gradient.setColorAt(1, Qt::black);

            QBrush gradientBrush{gradient};
            gradientBrush.setStyle(Qt::BrushStyle::LinearGradientPattern);
            painter.setBrush(gradientBrush);
            painter.drawRect(pinRect);

            xOffset += imageWidth / columns;
        }
    }
    return true;
}

bool makeACTImpl(QImage& image, int rows, int columns)
{
    QPainter painter;
    if (image.width() <= 0 || image.height() <= 0 || rows <= 0 || columns <= 0 ||
        !painter.begin(&image)) {
        return false;
    }

    return drawACTRows(painter, image.width(), image.height(), rows) &&
           drawACTColumns(painter, image.width(), image.height(), rows, columns);
}

} // namespace


bool CalibrationFactory::makeRGB(const QString &filePath, int imageWidth, int imageHeight, int rows, int columns)
{
    QImage image {imageWidth, imageHeight,  QImage::Format_ARGB32};
    image.fill(Qt::transparent);
    if (!makeRGBImpl(image, rows, columns)) {
        return false;
    }
    return image.save(filePath);;
}

bool CalibrationFactory::makeACT(const QString &filePath, int imageWidth, int imageHeight, int rows, int columns)
{
    QImage image {imageWidth, imageHeight,  QImage::Format_ARGB32};
    image.fill(Qt::transparent);
    if (!makeACTImpl(image, rows, columns)) {
        return false;
    }
    return image.save(filePath);;
}
