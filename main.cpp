#include <QGuiApplication>
#include <QCommandLineParser>
#include "CalibrationFactory.h"

#include <QDebug>

namespace Keywords {
    const QString t = "t";
    const QString w = "width";
    const QString h = "height";
    const QString rgb = "rgb";
}

struct RGBParams {
    int rows = 0;
    int columns = 0;
};

RGBParams getRGBType(const QString& rgbStr, bool& ok) {
    ok = false;
    RGBParams result;
    if (!rgbStr.startsWith(Keywords::rgb)) {
        return result;
    }

    QStringList args = rgbStr.mid(Keywords::rgb.size()).split("x", Qt::SkipEmptyParts);
    if (args.size() != 2) {
        return result;
    }

    result.columns = args.at(0).toInt(&ok);
    if (!ok || result.columns <= 0) {
        return result;
    }

    result.rows = args.at(1).toInt(&ok);
    ok &= result.rows > 0;
    return result;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("CalibrationImageFactory");
    QGuiApplication::setApplicationVersion("1.0");

    QString type = "rgb3x4";
    int width = 3840;
    int height = 2880;

    QCommandLineParser parser;
    parser.setApplicationDescription("Produces calibration image");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("destination", "File path to save image e.g /path/logo.png");
    parser.addOption({Keywords::t, QString{"Calibration image type. Format 'rgbCxR' C - columns, R - rows. Default %1"}.arg(type), "string", type});
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

    type = parser.value(Keywords::t);
    bool rgbOk = true;
    RGBParams rgb = getRGBType(type, rgbOk);
    const QString filePath = parser.positionalArguments().at(0);

    if (rgbOk) {
        if (!CalibrationFactory::makeRGB(filePath, width, height, rgb.rows, rgb.columns)) {
            qWarning() << "RGB image creation failed";
            return EXIT_FAILURE;
        }
        qDebug() << "Success. Please find rgb image at " << filePath;
        return EXIT_SUCCESS;
    }

    qWarning() << "Bad calibration type provided";
    return EXIT_FAILURE;
}
