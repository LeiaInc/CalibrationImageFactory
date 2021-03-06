#include <QGuiApplication>
#include <QCommandLineParser>
#include "CalibrationFactory.h"

#include <QDebug>
#include <QFileInfo>

namespace Keywords {
    const QString t = "t";
    const QString type = "type";
    const QString w = "width";
    const QString h = "height";
    const QString rgb = "rgb";
    const QString act = "act";
    const QString abar = "abar";
}

enum class PatternType {
    RGB = CalibrationFactory::PatternType::RGB,
    ACT = CalibrationFactory::PatternType::ACT,
    ALIGN_BAR = CalibrationFactory::PatternType::ALIGN_BAR,
    UNKNOWN
};

struct SizeParams {
    int rows = -1;
    int columns = -1;
    int viewsNumber = -1;
};

bool tryParsePattern(const QString& str, const QString& expectedPrefix, SizeParams& size) {
    if (!str.startsWith(expectedPrefix, Qt::CaseInsensitive)) {
        return false;
    }

    bool ok = true;
    const int viewsNumber = str.mid(expectedPrefix.size()).toInt(&ok);
    if (ok && viewsNumber > 0) {
        size.viewsNumber = viewsNumber;
        return true;
    }

    QStringList args = str.mid(expectedPrefix.size()).split("x", Qt::SkipEmptyParts);
    if (args.size() != 2) {
        return false;
    }

    size.columns = args.at(0).toInt(&ok);
    if (!ok || size.columns <= 0) {
        return false;
    }

    size.rows = args.at(1).toInt(&ok);
    return ok && size.rows > 0;
};

PatternType getPatternType(const QString& str, SizeParams& size) {
    static const auto patternMap = std::vector<std::pair<QString, PatternType>>{
        {Keywords::rgb, PatternType::RGB}, {Keywords::act, PatternType::ACT},
        {Keywords::abar, PatternType::ALIGN_BAR}
    };

    for (const auto& pair : patternMap) {
        if (tryParsePattern(str, pair.first, size)) {
            return pair.second;
        }
    }
    return PatternType::UNKNOWN;
}

void makeViews(const QString& baseName, const std::vector<QImage>& images)
{
    QFileInfo info {baseName};
    const char DOT = '.';


    const QString extension = info.completeSuffix().size() > 0 ? DOT + info.completeSuffix() : "";

    for (size_t i = 0; i < images.size(); ++i)
    {
        QString imageName = baseName.mid(0, baseName.size() - extension.size()) +
                QString::number(i) + extension;

        images[i].save(imageName);
        qDebug() << "Success. Please find image at " << imageName;
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("CalibrationImageFactory");
    QGuiApplication::setApplicationVersion("1.0");

    QString typeStr = "rgb3x4";
    int width = 3840;
    int height = 2880;

    QCommandLineParser parser;
    parser.setApplicationDescription("Produces calibration image");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("destination", "File path to save image e.g /path/logo.png");
    parser.addOption({{Keywords::t, Keywords::type}, QString{"Calibration image type. Format '{%1, %2, %3}CxR' C - columns, R - rows. Default %4"}.arg(Keywords::rgb, Keywords::act, Keywords::abar, typeStr), "string", typeStr});
    parser.addOption({Keywords::w, QString{"Calibration image width > 0. Default %1"}.arg(width), "positive int", QString::number(width)});
    parser.addOption({Keywords::h, QString{"Calibration image height > 0. Default %1"}.arg(height), "positive int", QString::number(height)});
    parser.process(app);

    if (parser.positionalArguments().size() <= 0) {
        qWarning() << "Image destination path is expected";
        return EXIT_FAILURE;
    }

    bool ok = true;
    width = parser.value(Keywords::w).toInt(&ok);
    if (!ok || width <= 0) {
        qWarning() << "Width should be positive integer";
        return EXIT_FAILURE;
    }

    height = parser.value(Keywords::h).toInt(&ok);
    if (!ok || height <= 0) {
        qWarning() << "Height should be positive integer ";
        return EXIT_FAILURE;
    }

    const QString filePath = parser.positionalArguments().at(0);
    SizeParams size;
    auto type = getPatternType(parser.value(Keywords::t), size);
    if (size.viewsNumber > 0 && type != PatternType::UNKNOWN) {
        makeViews(filePath, CalibrationFactory::getPattern(
             static_cast<CalibrationFactory::PatternType>(type),
                      width, height, size.viewsNumber));

        return EXIT_SUCCESS;
    }

    switch (type) {
        case PatternType::ACT:
            if (!CalibrationFactory::makeACT(filePath, width, height, size.rows, size.columns)) {
                qWarning() << "ACT image creation failed";
                return EXIT_FAILURE;
            }
            break;

        case PatternType::RGB:
            if (!CalibrationFactory::makeRGB(filePath, width, height, size.rows, size.columns)) {
                qWarning() << "RGB image creation failed";
                return EXIT_FAILURE;
            }
            break;

        case PatternType::ALIGN_BAR:
            if (!CalibrationFactory::makeABar(filePath, width, height, size.rows, size.columns)) {
                qWarning() << "Alignment bar image creation failed";
                return EXIT_FAILURE;
            }
            break;

        default:
            qWarning() << "Bad calibration type provided";
            return EXIT_FAILURE;
    }

    qDebug() << "Success. Please find image at " << filePath;
    return EXIT_SUCCESS;
}
