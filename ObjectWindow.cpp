#include "ObjectWindow.h"
#include "ui_ObjectWindow.h"

ObjectWindow::ObjectWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ObjectWindow)
{
    ui->setupUi(this);

    QRect scr = QApplication::primaryScreen()->geometry();
    move(scr.center() - rect().center());

    ui->dateEdit->setDate(QDate::currentDate());

    QRegularExpression regular;
    regular.setPattern("[^/?*:;{}\\\\]+");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regular, this);

    ui->lineEdit_object->setValidator(validator);
    ui->lineEdit_manufactory->setValidator(validator);
    ui->lineEdit_department->setValidator(validator);

    connect(ui->pushButton, &QPushButton::clicked, this, &ObjectWindow::ButtonClick);
}

void ObjectWindow::LoadFromReg(Registry *registry)
{
    m_registry = registry;
    m_objectInfo = registry->getObjectInfo();
    ui->lineEdit_object->setText(m_objectInfo->object);
    ui->lineEdit_manufactory->setText(m_objectInfo->manufactory);
    ui->lineEdit_department->setText(m_objectInfo->department);
    ui->lineEdit_FIO->setText(m_objectInfo->FIO);
}

ObjectWindow::~ObjectWindow()
{
    delete ui;
}

void ObjectWindow::ButtonClick()
{
    if (ui->lineEdit_object->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование объекта");
        return;
    }

    if (ui->lineEdit_manufactory->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование цеха");
        return;
    }

    if (ui->lineEdit_department->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите наименование отделение/установки");
        return;
    }

    if (!m_registry->checkObject(ui->lineEdit_object->text())) {
        QMessageBox::StandardButton button
            = QMessageBox::question(this,
                                    "Предупреждение",
                                    "Объекта нет в базе. Вы действительно хотите продождить?");

        if (button == QMessageBox::StandardButton::Yes) {
            SaveObjectInfo();

            accept();
        }

        return;
    }

    m_objectInfo->object = ui->lineEdit_object->text();

    if (!m_registry->checkManufactory(ui->lineEdit_manufactory->text())) {
        QMessageBox::StandardButton button
            = QMessageBox::question(this,
                                    "Предупреждение",
                                    "Цеха нет в базе. Вы действительно хотите продождить?");

        if (button == QMessageBox::StandardButton::Yes) {
            SaveObjectInfo();

            accept();
        }

        return;
    }

    m_objectInfo->manufactory = ui->lineEdit_manufactory->text();

    if (!m_registry->checkDepartment(ui->lineEdit_department->text())) {
        QMessageBox::StandardButton button = QMessageBox::question(
            this,
            "Предупреждение",
            "Отделения/установки нет в базе. Вы действительно хотите продождить?");

        if (button == QMessageBox::StandardButton::Yes) {
            SaveObjectInfo();

            accept();
        }

        return;
    }

    OtherParameters *otherParameters = m_registry->getOtherParameters();
    otherParameters->date = ui->dateEdit->date().toString("dd.MM.yyyy");
    SaveObjectInfo();
    accept();
}

void ObjectWindow::SaveObjectInfo()
{
    m_objectInfo->object = ui->lineEdit_object->text();
    m_objectInfo->manufactory = ui->lineEdit_manufactory->text();
    m_objectInfo->department = ui->lineEdit_department->text();
    m_objectInfo->FIO = ui->lineEdit_FIO->text();

    m_registry->saveObjectInfo();
}
