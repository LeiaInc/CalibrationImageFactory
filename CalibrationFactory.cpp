#include "CalibrationFactory.h"

#include <vector>

#include <QImage>
#include <QPainter>
#include <QFontMetrics>

namespace {

std::vector<QColor> getColorsSet(size_t size)
{
    std::vector<QColor> colorSet;
    colorSet.reserve(size);

    const std::vector<QColor> predefined = {
        QColor{Qt::red},
        QColor{Qt::green},
        QColor{Qt::blue},
        QColor{Qt::yellow},
        QColor{Qt::magenta},
        QColor{Qt::gray},
        QColor{Qt::cyan}
    };

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
    const auto columnColors = getColorsSet(columns);
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
