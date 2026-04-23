#include "Saver.h"

#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QOpenGLWidget>
#include <QPainter>
#include <QDataStream>
#include <QDebug>
#include <QDirIterator>

#include "xlsxdocument.h"
#include "xlsxdatavalidation.h"

using namespace QXlsx;

namespace Report {

Saver::Saver(QObject* parent)
    : QObject(parent)
{
}

Saver::~Saver()
{
    cleanupEmptyDirectory();
}

const QDir& Saver::directory() const
{
    return m_dir;
}

void Saver::setBasePath(const QString& basePath)
{
    if (m_isDirectoryCreated) {
        qWarning() << "Saver: base path is ignored because directory is already created:"
                   << m_dir.path();
        return;
    }

    m_basePath = basePath;
}

bool Saver::ensureDirectory()
{
    if (m_isDirectoryCreated)
        return true;

    if (createAutoDirectory())
        return true;

    return chooseDirectoryManually();
}

bool Saver::createAutoDirectory()
{
    const QString dateFolder =
        QDate::currentDate().toString(QStringLiteral("dd_MM_yyyy"));

    QDir baseDir(QCoreApplication::applicationDirPath());

    QString rootPath = baseDir.path();
    if (!m_basePath.isEmpty())
        rootPath = baseDir.filePath(m_basePath);

    QDir rootDir(rootPath);

    if (!rootDir.exists()) {
        if (!QDir().mkpath(rootPath))
            return false;
        rootDir.setPath(rootPath);
    }

    if (rootDir.exists(dateFolder)) {
        for (int i = 2; i < 50; ++i) {
            const QString folderName = QStringLiteral("%1_%2")
                                           .arg(dateFolder)
                                           .arg(i);
            if (rootDir.mkdir(folderName)) {
                m_dir = QDir(rootDir.filePath(folderName));
                m_isDirectoryCreated = true;
                return true;
            }
        }
        return false;
    }

    if (!rootDir.mkdir(dateFolder))
        return false;

    m_dir = QDir(rootDir.filePath(dateFolder));
    m_isDirectoryCreated = true;
    return true;
}

bool Saver::chooseDirectoryManually()
{
    QDir startDir = m_isDirectoryCreated
        ? m_dir
        : QDir(QCoreApplication::applicationDirPath());

    while (!m_isDirectoryCreated) {
        QString folderPath;

        do {
            emit setDirectoryToSave(startDir.path(), folderPath);
        } while (folderPath.isEmpty());

        QDir chosenDir(folderPath);

        if (!chosenDir.exists()) {
            if (!QDir().mkpath(folderPath))
                continue;
            chosenDir.setPath(folderPath);
        }

        if (!chosenDir.isEmpty()) {
            bool answer = emit question(
                QStringLiteral("Внимание!"),
                QStringLiteral("Папка не пуста. Вы действительно хотите выбрать эту папку?"));

            if (!answer)
                continue;
        }

        m_dir = chosenDir;
        m_isDirectoryCreated = true;
    }

    return true;
}

void Saver::saveImage(Widgets::Chart::ChartView* chart)
{
    if (!chart)
        return;

    if (!ensureDirectory())
        return;

    const QString name = chart->getname();
    const quint16 index = ++m_chartCounter[name];

    QPixmap pixmap = chart->grab();

    if (auto* glWidget = chart->findChild<QOpenGLWidget*>()) {
        QPainter painter(&pixmap);
        const QPoint delta =
            glWidget->mapToGlobal(QPoint()) - chart->mapToGlobal(QPoint());
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawImage(delta, glWidget->grabFramebuffer());
    }

    const QString baseName =
        QStringLiteral("%1_%2").arg(name).arg(index);

    const QString bmpPath =
        m_dir.filePath(baseName + QStringLiteral(".bmp"));
    pixmap.save(bmpPath);

    QFile out(m_dir.filePath(baseName + QStringLiteral(".data")));
    if (out.open(QIODevice::WriteOnly)) {
        QDataStream stream(&out);
        stream.setVersion(QDataStream::Qt_6_2);
        chart->saveToStream(stream);
        out.flush();
        out.close();
    }
}

bool Saver::saveReport(const Report& report, const QString& templatePath)
{
    if (!ensureDirectory())
        return false;

    const QString tempTemplatePath =
        m_dir.filePath(QStringLiteral("_template_copy.xlsx"));

    QFile::remove(tempTemplatePath);

    if (!QFile::copy(templatePath, tempTemplatePath)) {
        qWarning() << "Не удалось скопировать шаблон:"
                   << templatePath << "->" << tempTemplatePath;
        return false;
    }

    Document xlsx(tempTemplatePath);

    for (const auto& item : report.data) {
        if (!xlsx.selectSheet(item.sheet)) {
            qWarning() << "Не найден лист" << item.sheet;
            continue;
        }

        if (item.row <= 0 || item.col <= 0) {
            qWarning() << "Невалидные координаты ячейки"
                       << item.row << item.col;
            continue;
        }

        xlsx.write(item.row, item.col, item.value);
    }

    constexpr int targetWidth = 885;
    constexpr int targetHeight = 460;

    for (const auto& img : report.images) {
        if (img.image.isNull())
            continue;

        if (!xlsx.selectSheet(img.sheet)) {
            qWarning() << "Не найден лист" << img.sheet;
            continue;
        }

        if (img.row <= 0 || img.col <= 0) {
            qWarning() << "Невалидные координаты изображения"
                       << img.row << img.col;
            continue;
        }

        const QImage scaled = img.image.scaled(
            targetWidth,
            targetHeight,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);

        xlsx.insertImage(img.row, img.col, scaled);
    }

    for (const auto& v : report.validation) {
        DataValidation validation(
            DataValidation::List,
            DataValidation::Equal,
            v.formula);
        validation.addRange(v.range);
        xlsx.addDataValidation(validation);
    }

    const QString reportPath =
        m_dir.filePath(QStringLiteral("report.xlsx"));

    const bool ok = xlsx.saveAs(reportPath);

    QFile::remove(tempTemplatePath);

    return ok && QFile::exists(reportPath);
}

void Saver::cleanupEmptyDirectory()
{
    if (!m_isDirectoryCreated)
        return;

    if (!m_dir.exists())
        return;

    const QFileInfoList entries = m_dir.entryInfoList(
        QDir::NoDotAndDotDot | QDir::AllEntries);

    if (!entries.isEmpty())
        return;

    const QString path = m_dir.absolutePath();
    QDir parent = m_dir;
    parent.cdUp();
    parent.rmdir(QFileInfo(path).fileName());
}

}