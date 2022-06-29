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
    QColor{Qt::blue}
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

void drawRGBMatrix(QPainter& painter, const QRectF& boundRect, size_t matrixSize, const QString& text)
{
    const qreal rows = matrixSize;
    const qreal columns = matrixSize;
    const qreal textPixelSize = qMax(5.0, qMin(90.0, qMin(boundRect.width(), boundRect.height()) / (matrixSize * 2 - 1)));
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
            int allignV = Qt::AlignVCenter;
            int allignH = Qt::AlignHCenter;

            if (i == 0) {
                allignV = Qt::AlignTop;
            } else if (i == rows - 1) {
                allignV = Qt::AlignBottom;
            }

            if (j == 0) {
                allignH = Qt::AlignLeft;
            } else if (j == columns - 1) {
                allignH = Qt::AlignRight;
            }

            painter.drawText(textRect, allignH | allignV, text);
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

    const auto margin = 5;
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

            drawRGBMatrix(painter,
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
    painter.setPen(Qt::transparent);

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

    const qreal pinWidth = 10;
    const qreal pinWidthHalf = pinWidth / 2;
    const qreal pinHeightRatio = 0.45;
    const int stripesNumber = rows * static_cast<int>(ACT_SET.size());
    const qreal stripeHeight = imageHeight / stripesNumber;
    const qreal pinHeight = stripeHeight * pinHeightRatio;
    const qreal pinYOffset = stripeHeight * (1 - pinHeightRatio) / 2;

    const qreal pinsHSpace = 2;
    const qreal pinsVSpace = stripeHeight * 0.05;
    const qreal hDelta = pinWidth + pinsHSpace;
    const qreal leftPos = imageWidth / columns / 2 - hDelta * (columns * rows - 1) / 2;

    qreal yOffset = 0;
    qreal xOffset = 0;
    unsigned counter = 0;

    painter.setPen(Qt::transparent);

    for (int i = 0; i < rows; ++i) {
        yOffset = i * stripeHeight * ACT_SET.size();

        for(int j = 0; j < columns; ++j) {
            xOffset = leftPos + counter++ * hDelta + j * imageWidth / columns;
            const qreal xPos = qBound(pinWidthHalf, xOffset, imageWidth - pinWidthHalf);

            for (int k = 0; k < static_cast<int>(ACT_SET.size()); ++k) {
                qreal yPinPos = yOffset + pinYOffset + stripeHeight * k;
                if (k == 0) {
                    yPinPos = yOffset + stripeHeight - pinHeight - pinsVSpace;
                } else if (k == static_cast<int>(ACT_SET.size()) - 1) {
                    yPinPos = yOffset + pinsVSpace + stripeHeight * k;
                }

                painter.setBrush(QBrush{QColor{0, 255, 0}});
                QRectF pinRect{xPos - pinWidthHalf, yPinPos, pinWidth, pinHeight};
                painter.drawRect(pinRect);

                QLinearGradient gradient {pinRect.topLeft(), pinRect.bottomRight()};
                gradient.setColorAt(0, Qt::transparent);
                gradient.setColorAt(1, Qt::black);

                QBrush gradientBrush{gradient};
                gradientBrush.setStyle(Qt::BrushStyle::LinearGradientPattern);
                painter.setBrush(gradientBrush);
                painter.drawRect(pinRect);
            }
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

//////////////////////////// ALIGN BAR
void drawAbarGrid(QPainter& painter, QRectF boundingRect, int gridRows, int gridColumns, int barIndex)
{
    const qreal text2BoundingRectMargin = 5;
    const qreal fontPixelSize = qMax(10.0, qMin(boundingRect.width() / gridColumns, boundingRect.height() / gridRows));
    const qreal gridCellWidth = boundingRect.width() / gridColumns;
    const qreal gridCellHeight = (boundingRect.height() - fontPixelSize - text2BoundingRectMargin) / gridRows;
    const qreal penWidth = 1.0;
    QFont font {"Arrial"};
    font.setPixelSize(fontPixelSize);

    const QString str = QString::number(barIndex);
    const QSizeF textSize = QFontMetrics{font}.size(Qt::TextSingleLine, str);
    qreal xCell = 0;
    qreal yCell = 0;

    // draw grid cells with background
    for (int k = 0; k < gridRows; ++k) {
        xCell = k == 0 ? boundingRect.x() + gridCellWidth / 2 : boundingRect.x();
        yCell = boundingRect.y() + k * gridCellHeight;

        // draw cell background
        painter.setPen(QPen{});

        // draw colored 1.5 cels
        if (k == 0) {
            painter.setBrush(QColor{83, 177, 54});

            // both cases (i - 0.5) % n
            if (barIndex == 0) { // 0.5 + n.5
                painter.drawRect(QRectF{xCell,
                                        boundingRect.y(), gridCellWidth, gridCellHeight});

                painter.drawRect(QRectF{xCell + (gridColumns - 1) * gridCellWidth,
                                        boundingRect.y(), gridCellWidth, gridCellHeight});
            } else { // two big
                painter.drawRect(QRectF{xCell + (barIndex - 1) * gridCellWidth,
                                        boundingRect.y(), gridCellWidth * 2, gridCellHeight});
            }
        } else { // one colored cell
            painter.setBrush(QColor{117, 251, 76});
            QRectF rect {xCell + (barIndex) * gridCellWidth, yCell, gridCellWidth, gridCellHeight};
            painter.drawRect(rect);

            QRectF textRect = {rect.bottomLeft(), textSize};
            textRect.moveCenter(QPointF{rect.center().x(),  textRect.center().y()});
            textRect.setX(qBound(boundingRect.left(), textRect.x(), boundingRect.right() - textSize.width()));

            painter.setFont(font);
            painter.setPen(painter.brush().color());
            painter.drawText(textRect, Qt::AlignHCenter|Qt::AlignTop, str);
        }

        for (int z = 0; z < gridColumns; ++z) {
            // draw grid cells
            painter.setBrush(QBrush{Qt::transparent});
            painter.setPen(QPen{QBrush{QColor{122, 122, 122}}, penWidth});
            painter.drawRect(QRectF{xCell, yCell, gridCellWidth, gridCellHeight});
            xCell += gridCellWidth;
        }
    }
}

void drawAbarMatrix(QPainter& painter, QRectF boundRect, int barIndex, int barSize)
{
    const qreal rows = 3;
    const qreal columns = 3;
    qreal xOffset = 0;
    qreal yOffset = 0;

    const qreal sideMargin = 40;
    QRectF matrixRect;
    matrixRect.setWidth(boundRect.width() - sideMargin * 2);
    matrixRect.setHeight(boundRect.height() - sideMargin * 2);
    matrixRect.moveCenter(boundRect.center());

    const int gridRows = 2;
    const int gridColumns = barSize;
    const qreal tileHeight = matrixRect.height() / rows;
    const qreal tileWidth = matrixRect.width() / columns;

    const qreal gridWidth = qMin(tileWidth, 12.0 * gridColumns);
    const qreal gridHeight = qMin(tileHeight, 50.0 * gridRows);

    for (size_t i = 0; i < rows; ++i) {
        xOffset = 0;
        for (size_t j = 0; j < columns; ++j) {
            QRectF gridRect {matrixRect.left() + xOffset, matrixRect.top() + yOffset,
                            gridWidth, gridHeight};

            if (j == columns - 1) {
                gridRect.moveRight(matrixRect.right());
            } else if (j != 0) {
                gridRect.moveLeft(gridRect.left() - gridRect.width() / 2);
            }

            if (i == rows - 1) {
                gridRect.moveBottom(matrixRect.bottom());
            } else if (i != 0) {
                gridRect.moveTop(gridRect.top() - gridRect.height() / 2);
            }


            drawAbarGrid(painter, gridRect, gridRows, gridColumns, barIndex);
            xOffset += matrixRect.width() / (columns - 1);
        }
        yOffset += matrixRect.height() / (rows - 1);
    }
}

bool makeABarImpl(QImage& image, int rows, int columns)
{
    image.fill(Qt::black);
    QPainter painter;
    if (image.width() <= 0 || image.height() <= 0 || rows <= 0 || columns <= 0 ||
        !painter.begin(&image)) {
        return false;
    }

    const qreal tileHeight = static_cast<qreal>(image.height()) / rows;
    const qreal tileWidth = static_cast<qreal>(image.width()) / columns;

    qreal xOffset = 0;
    qreal yOffset = 0;
    int value = 0;

    for (int i = 0; i < rows; ++i) {
        yOffset = tileHeight * i;

        for (int j = 0; j < columns; ++j) {
            xOffset = tileWidth * j;
            drawAbarMatrix(painter, QRectF{xOffset, yOffset, tileWidth, tileHeight}, value++, rows * columns);
        }
    }
    return true;
}

std::vector<QImage> splitImage(const QImage& image, qreal tileWidth, qreal tileHeight, int number)
{
    std::vector<QImage> result;
    for (int i = 0; i < number; ++i) {
        result.emplace_back(image.copy(tileWidth * i, 0, tileWidth, tileHeight));
    }
    return result;
}

} // namespace


bool CalibrationFactory::makeRGB(const QString &filePath, int imageWidth, int imageHeight, int rows, int columns)
{
    QImage image {imageWidth, imageHeight,  QImage::Format_ARGB32};
    image.fill(Qt::transparent);
    if (!makeRGBImpl(image, rows, columns)) {
        return false;
    }
    return image.save(filePath);
}

bool CalibrationFactory::makeACT(const QString &filePath, int imageWidth, int imageHeight, int rows, int columns)
{
    QImage image {imageWidth, imageHeight,  QImage::Format_ARGB32};
    image.fill(Qt::transparent);
    if (!makeACTImpl(image, rows, columns)) {
        return false;
    }
    return image.save(filePath);
}

bool CalibrationFactory::makeABar(const QString &filePath, int imageWidth, int imageHeight, int rows, int columns)
{
    QImage image {imageWidth, imageHeight,  QImage::Format_ARGB32};
    if (!makeABarImpl(image, rows, columns)) {
        return false;
    }
    return image.save(filePath);
}

bool CalibrationFactory::makePattern(CalibrationFactory::PatternType type, const QString &filePath, int imageWidth, int imageHeight, int rows, int columns)
{
    QImage image {imageWidth, imageHeight,  QImage::Format_ARGB32};
    image.fill(Qt::transparent);

    bool result{false};
    switch (type)
    {
    case CalibrationFactory::RGB:
        result = makeRGBImpl(image, rows, columns);
        break;
    case CalibrationFactory::ACT:
        result = makeACTImpl(image, rows, columns);
        break;
    case CalibrationFactory::ALIGN_BAR:
        result = makeABarImpl(image, rows, columns);
        break;
    }

    return result && image.save(filePath);
}

std::vector<QImage> CalibrationFactory::getPattern(CalibrationFactory::PatternType type, int width, int height, int number)
{
    if (width <= 0 || height <= 0 || number <= 0) {
        return {};
    }

    QImage image {width * number, height,  QImage::Format_ARGB32};
    image.fill(Qt::transparent);
    switch (type) {
        case CalibrationFactory::RGB:
            makeRGBImpl(image, 1, number);
            break;

        case CalibrationFactory::ALIGN_BAR:
            makeABarImpl(image, 1, number);
            break;

        case CalibrationFactory::ACT:
            makeACTImpl(image, 1, number);
            break;

        default: return {};
    }

    return splitImage(image, width, height, number);
}
