#include <QMessageBox>
#include <QScreen>

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
    registry_ = registry;
    object_info_ = registry->GetObjectInfo();
    ui->lineEdit_object->setText(object_info_->object);
    ui->lineEdit_manufactory->setText(object_info_->manufactory);
    ui->lineEdit_department->setText(object_info_->department);
    ui->lineEdit_FIO->setText(object_info_->FIO);
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

    if (!registry_->CheckObject(ui->lineEdit_object->text())) {
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

    object_info_->object = ui->lineEdit_object->text();

    if (!registry_->CheckManufactory(ui->lineEdit_manufactory->text())) {
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

    object_info_->manufactory = ui->lineEdit_manufactory->text();

    if (!registry_->CheckDepartment(ui->lineEdit_department->text())) {
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

    OtherParameters *other_parameters = registry_->GetOtherParameters();
    other_parameters->data = ui->dateEdit->date().toString("dd.MM.yyyy");
    SaveObjectInfo();
    accept();
}

void ObjectWindow::SaveObjectInfo()
{
    object_info_->object = ui->lineEdit_object->text();
    object_info_->manufactory = ui->lineEdit_manufactory->text();
    object_info_->department = ui->lineEdit_department->text();
    object_info_->FIO = ui->lineEdit_FIO->text();

    registry_->SaveObjectInfo();
}
