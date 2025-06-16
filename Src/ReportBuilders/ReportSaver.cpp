#include "ReportSaver.h"
#include "xlsxdatavalidation.h"
#include "xlsxdocument.h"

using namespace QXlsx;

ReportSaver::ReportSaver(QObject *parent)
    : QObject{parent}
{
    m_created = false;
}

void ReportSaver::SetRegistry(Registry *registry)
{
    m_registry = registry;
}

void ReportSaver::SaveImage(MyChart *chart)
{
    if (!m_created)
        CreateDir();

    static QMap<QString, quint16> chartNum;

    QString name = chart->getname();

    ++chartNum[name];

    QPixmap p = chart->grab();
    QOpenGLWidget *glWidget = chart->findChild<QOpenGLWidget *>();

    if (glWidget) {
        QPainter painter(&p);
        QPoint d = glWidget->mapToGlobal(QPoint()) - chart->mapToGlobal(QPoint());
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawImage(d, glWidget->grabFramebuffer());
        painter.end();
    }

    p.save(m_dir.filePath(name + "_" + QString::number(chartNum[name]) + ".bmp"));

    QFile out(m_dir.filePath(name + "_" + QString::number(chartNum[name]) + ".data"));

    if (out.open(QIODevice::WriteOnly)) {
        QDataStream stream(&out);
        stream.setVersion(QDataStream::Qt_6_2);
        chart->loadFromStream(stream);
        out.flush();
        out.close();
    }
}

QDir ReportSaver::Directory()
{
    return m_dir.path();
}

bool ReportSaver::SaveReport(const Report &report, const QString &templatePath)
{
    if (!m_created)
        CreateDir();

    Document xlsx(templatePath);

    for (const auto &data : report.data) {
        if (!xlsx.selectSheet(data.sheet)) {
            qWarning() << "Не найден лист" << data.sheet << "для записи данных!";
            continue;
        }
        xlsx.write(data.x, data.y, data.value);
    }

    for (const auto& img : report.images) {
        if (!img.image.isNull() && xlsx.selectSheet(img.sheet)) {
            xlsx.insertImage(img.row, img.col, img.image);
        }
    }

    for (const auto &valid : report.validation) {
        DataValidation validation(DataValidation::List, DataValidation::Equal, valid.formula);
        validation.addRange(valid.range);

        xlsx.addDataValidation(validation);
    }

    xlsx.saveAs(m_dir.filePath("report.xlsx"));

    return QFile::exists(m_dir.filePath("report.xlsx"));
}

void ReportSaver::CreateDir()
{
    ObjectInfo *objectInfo = m_registry->GetObjectInfo();
    ValveInfo *valveInfo = m_registry->GetValveInfo();

    QString path = objectInfo->object + "/" + objectInfo->manufactory + "/"
                   + objectInfo->department + "/" + valveInfo->positionNumber;
    QString date = QDate::currentDate().toString("dd_MM_yyyy");

    m_dir.setPath(QCoreApplication::applicationDirPath());

    if (m_dir.mkpath(path)) {
        m_dir.cd(path);

        if (m_dir.exists(date)) {
            for (int i = 2; i < 50; i++) {
                QString folder = date + "_" + QString::number(i);

                if (m_dir.mkdir(folder)) {
                    m_created = m_dir.cd(folder);
                    break;
                }
            }
        } else if (m_dir.mkdir(date)) {
            m_created = m_dir.cd(date);
        }
    }

    while (!m_created) {
        QString folder;

        do {
            emit GetDirectory(m_dir.path(), folder);
        } while (folder.isEmpty());

        if (m_dir.cd(folder)) {
            if (m_dir.isEmpty()) {
                m_created = m_dir.cd(folder);
            } else {
                bool answer;
                emit Question("Внимание!",
                              "Папка не пуста. Вы действительно хотите выбрать эту папку?",
                              answer);

                if (answer) {
                    m_created = m_dir.cd(folder);
                }
            }
        }
    }
}
