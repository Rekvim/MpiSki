#include "FileSaver.h"
#include "xlsxdatavalidation.h"
#include "xlsxdocument.h"

using namespace QXlsx;

FileSaver::FileSaver(QObject *parent)
    : QObject{parent}
{
    created_ = false;
}

void FileSaver::SetRegistry(Registry *registry)
{
    registry_ = registry;
}

void FileSaver::SaveImage(MyChart *chart)
{
    if (!created_)
        CreateDir();

    static QMap<QString, quint16> chart_num;

    QString name = chart->getname();

    ++chart_num[name];

    QPixmap p = chart->grab();
    QOpenGLWidget *glWidget = chart->findChild<QOpenGLWidget *>();

    if (glWidget) {
        QPainter painter(&p);
        QPoint d = glWidget->mapToGlobal(QPoint()) - chart->mapToGlobal(QPoint());
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawImage(d, glWidget->grabFramebuffer());
        painter.end();
    }

    p.save(dir_.filePath(name + "_" + QString::number(chart_num[name]) + ".bmp"));

    QFile out(dir_.filePath(name + "_" + QString::number(chart_num[name]) + ".data"));

    if (out.open(QIODevice::WriteOnly)) {
        QDataStream stream(&out);
        stream.setVersion(QDataStream::Qt_6_2);
        chart->savetostream(stream);
        out.flush();
        out.close();
    }
}

QDir FileSaver::Directory()
{
    return dir_.path();
}

bool FileSaver::SaveReport(const Report &report)
{
    if (!created_)
        CreateDir();

    Document xlsx(":/excel/report.xlsx");

    for (const auto &data : report.data) {
        xlsx.write(data.x, data.y, data.line_edit->text());
    }

    if (!report.image1.isNull()) {
        xlsx.insertImage(86, 1, report.image1);
    }

    if (!report.image2.isNull()) {
        xlsx.insertImage(111, 1, report.image2);
    }

    if (!report.image3.isNull()) {
        xlsx.insertImage(136, 1, report.image3);
    }

    for (const auto &valid : report.validation) {
        DataValidation validation(DataValidation::List, DataValidation::Equal, valid.formula);
        validation.addRange(valid.range);

        xlsx.addDataValidation(validation);
    }

    xlsx.saveAs(dir_.filePath("report.xlsx"));

    return QFile::exists(dir_.filePath("report.xlsx"));
}

void FileSaver::CreateDir()
{
    ObjectInfo *object_info = registry_->GetObjectInfo();
    ValveInfo *valve_info = registry_->GetValveInfo();

    QString path = object_info->object + "/" + object_info->manufactory + "/"
                   + object_info->department + "/" + valve_info->position;
    QString date = QDate::currentDate().toString("dd_MM_yyyy");

    dir_.setPath(QCoreApplication::applicationDirPath());

    if (dir_.mkpath(path)) {
        dir_.cd(path);

        if (dir_.exists(date)) {
            for (int i = 2; i < 50; i++) {
                QString folder = date + "_" + QString::number(i);

                if (dir_.mkdir(folder)) {
                    created_ = dir_.cd(folder);
                    break;
                }
            }
        } else if (dir_.mkdir(date)) {
            created_ = dir_.cd(date);
        }
    }

    while (!created_) {
        QString folder;

        do {
            emit GetDirectory(dir_.path(), folder);
        } while (folder.isEmpty());

        if (dir_.cd(folder)) {
            if (dir_.isEmpty()) {
                created_ = dir_.cd(folder);
            } else {
                bool answer;
                emit Question("Внимание!",
                              "Папка не пуста. Вы действительно хотите выбрать эту папку?",
                              answer);

                if (answer) {
                    created_ = dir_.cd(folder);
                }
            }
        }
    }
}
