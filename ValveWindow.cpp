#include <QMessageBox>
#include "ValveWindow.h"
#include "ui_ValveWindow.h"

ValveWindow::ValveWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ValveWindow)
{
    ui->setupUi(this);

    // Позволяет вводить только цифры (0–9), пустая строка тоже валидна
    auto *numVal = new QRegularExpressionValidator(
        QRegularExpression(R"(^[0-9]*$)"), this);

    // A–Z, a–z, А–Я, а–я, Ёё, пустая строка тоже валидна
    auto *alphaRusVal = new QRegularExpressionValidator(
        QRegularExpression(R"(^[A-Za-zА-Яа-яЁё]*$)"), this);

    // Убераем символы
    QRegularExpression reNoBad(R"(^[^@!^\/\?\*\:\;\{\}\\]*$)");
    auto *noBadVal = new QRegularExpressionValidator(reNoBad, this);


    ui->lineEdit_positionNumber->setValidator(noBadVal);

    ui->lineEdit_manufacturer  ->setValidator(alphaRusVal);

    ui->lineEdit_serial                     ->setValidator(noBadVal);
    ui->lineEdit_DN                         ->setValidator(noBadVal);
    ui->lineEdit_PN                         ->setValidator(noBadVal);
    ui->lineEdit_valveStroke                ->setValidator(noBadVal);
    ui->lineEdit_dinamicError               ->setValidator(noBadVal);
    ui->lineEdit_modelDrive                 ->setValidator(noBadVal);
    ui->lineEdit_rangeDrive                 ->setValidator(noBadVal);
    ui->lineEdit_materialStuffingBoxSeal    ->setValidator(noBadVal);

    ui->doubleSpinBox_diameterPulley->setValue(diameter_[0]);

    connect(ui->comboBox_strokeMovement,
            &QComboBox::currentIndexChanged,
            this,
            &ValveWindow::StrokeChanged);
    connect(ui->comboBox_toolNumber,
            &QComboBox::currentIndexChanged,
            this,
            &ValveWindow::ToolChanged);
    connect(ui->doubleSpinBox_diameterPulley,
            &QDoubleSpinBox::valueChanged,
            this,
            &ValveWindow::DiameterChanged);

    connect(ui->pushButton, &QPushButton::clicked, this, &ValveWindow::ButtonClick);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &ValveWindow::Clear);

    DiameterChanged(ui->doubleSpinBox_diameterPulley->value());
}

ValveWindow::~ValveWindow()
{
    delete ui;
}

void ValveWindow::SetRegistry(Registry *registry)
{
    registry_ = registry;

    valve_info_ = registry_->GetValveInfo();


    // ui->comboBox_position->clear();
    // ui->comboBox_position->addItems(registry_->GetPositions());
    // ui->comboBox_position->addItem(manual_input_);

    // QString last_position = registry_->GetLastPosition();
    // if (last_position == "") {
    //     ui->comboBox_position->setCurrentIndex(ui->comboBox_position->count() - 1);
    // } else {
    //     ui->comboBox_position->setCurrentIndex(ui->comboBox_position->findText(last_position));
    //     PositionChanged(last_position);
    // }

    // connect(ui->comboBox_position,
    //         &QComboBox::currentTextChanged,
    //         this,
    //         &ValveWindow::PositionChanged);
}

void ValveWindow::SaveValveInfo()
{
    valve_info_->serial = ui->lineEdit_serial->text();
    valve_info_->DN = ui->lineEdit_DN->text();
    valve_info_->PN = ui->lineEdit_PN->text();
    valve_info_->stroke = ui->lineEdit_valveStroke->text();
    valve_info_->positioner = ui->lineEdit_positionNumber->text();
    valve_info_->dinamic_error = ui->lineEdit_dinamicError->text();
    valve_info_->model_drive = ui->lineEdit_modelDrive->text();
    valve_info_->range = ui->lineEdit_rangeDrive->text();
    valve_info_->material = ui->lineEdit_materialStuffingBoxSeal->text();
    valve_info_->manufacturer = ui->lineEdit_manufacturer->text();

    valve_info_->diameter = ui->doubleSpinBox_diameterPulley->value();
    valve_info_->pulley = ui->doubleSpinBox_diameterPulley->value();

    valve_info_->normal_position = ui->comboBox_safePosition->currentIndex();
    valve_info_->drive_type = ui->comboBox_typeDrive->currentIndex();
    valve_info_->stroke_movement = ui->comboBox_strokeMovement->currentIndex();
    valve_info_->tool_number = ui->comboBox_toolNumber->currentIndex();

    registry_->SaveValveInfo();
}

void ValveWindow::PositionChanged(const QString &position)
{
    if (position == "Ручной ввод") {
        ui->lineEdit_positionNumber->setEnabled(true);
        return;
    }

    ui->lineEdit_DN->setEnabled(false);

    valve_info_ = registry_->GetValveInfo(position);

    ui->lineEdit_positionNumber->setText(position);
    ui->lineEdit_positionNumber->setEnabled(false);

    ui->lineEdit_manufacturer->setText(valve_info_->manufacturer);
    ui->lineEdit_serial->setText(valve_info_->serial);
    ui->lineEdit_DN->setText(valve_info_->DN);
    ui->lineEdit_PN->setText(valve_info_->PN);
    ui->lineEdit_valveStroke->setText(valve_info_->stroke);
    ui->lineEdit_positionNumber->setText(valve_info_->positioner);
    ui->lineEdit_dinamicError->setText(valve_info_->dinamic_error);
    ui->lineEdit_modelDrive->setText(valve_info_->model_drive);
    ui->lineEdit_rangeDrive->setText(valve_info_->range);
    ui->lineEdit_materialStuffingBoxSeal->setText(valve_info_->material);

    ui->doubleSpinBox_diameterPulley->setValue(valve_info_->diameter);

    ui->comboBox_safePosition->setCurrentIndex(valve_info_->normal_position);
    ui->comboBox_typeDrive->setCurrentIndex(valve_info_->drive_type);
    ui->comboBox_strokeMovement->setCurrentIndex(valve_info_->stroke_movement);
    ui->comboBox_toolNumber->setCurrentIndex(valve_info_->tool_number);
    ui->doubleSpinBox_diameterPulley->setValue(valve_info_->pulley);
}

void ValveWindow::ButtonClick()
{
    if (ui->lineEdit_positionNumber->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите номер позиции");
        return;
    }

    // if ((ui->lineEdit_manufacturer->text().isEmpty()) or (ui->lineEdit_valve_model->text().isEmpty())
    //     or (ui->lineEdit_serial->text().isEmpty()) or (ui->lineEdit_DN->text().isEmpty())
    //     or (ui->lineEdit_PN->text().isEmpty()) or (ui->lineEdit_valveStroke->text().isEmpty())
    //     or (ui->lineEdit_positionNumberNumber->text().isEmpty())
    //     or (ui->lineEdit_dinamicError->text().isEmpty())
    //     or (ui->lineEdit_modelDrive->text().isEmpty()) or (ui->lineEdit_rangeDrive->text().isEmpty())
    //     or (ui->lineEdit_materialStuffingBoxSeal->text().isEmpty())) {
    //     QMessageBox::StandardButton button
    //         = QMessageBox::question(this,
    //                                 "Предупреждение",
    //                                 "Введены не все данные, вы действительно хотети продолжить?");

    //     if (button == QMessageBox::StandardButton::No) {
    //         return;
    //     }
    // }

    OtherParameters *other_parameters = registry_->GetOtherParameters();
    other_parameters->normal_position = ui->comboBox_safePosition->currentText();
    other_parameters->movement = ui->comboBox_strokeMovement->currentText();
    SaveValveInfo();

    accept();
}

void ValveWindow::StrokeChanged(quint16 n)
{
    ui->comboBox_toolNumber->setEnabled(n == 1);
    ui->doubleSpinBox_diameterPulley->setEnabled(
        (n == 1)
        && (ui->comboBox_toolNumber->currentIndex() == ui->comboBox_toolNumber->count() - 1));
}

void ValveWindow::ToolChanged(quint16 n)
{
    if (n == ui->comboBox_toolNumber->count() - 1) {
        ui->doubleSpinBox_diameterPulley->setEnabled(true);
    } else {
        ui->doubleSpinBox_diameterPulley->setEnabled(false);
        ui->doubleSpinBox_diameterPulley->setValue(diameter_[n]);
    }
}

void ValveWindow::DiameterChanged(qreal value)
{
    ui->label_valueSquare->setText(QString().asprintf("%.2f", M_PI * value * value / 4));
}

void ValveWindow::Clear()
{
    ui->lineEdit_manufacturer->setText("");
    // ui->lineEdit_valve_model->setText("");
    ui->lineEdit_serial->setText("");
    ui->lineEdit_DN->setText("");
    ui->lineEdit_PN->setText("");
    ui->lineEdit_valveStroke->setText("");
    ui->lineEdit_positionNumber->setText("");
    ui->lineEdit_dinamicError->setText("");
    ui->lineEdit_modelDrive->setText("");
    ui->lineEdit_rangeDrive->setText("");
    ui->lineEdit_materialStuffingBoxSeal->setText("");

    ui->doubleSpinBox_diameterPulley->setValue(1.0);

    ui->comboBox_safePosition->setCurrentIndex(0);
    ui->comboBox_typeDrive->setCurrentIndex(0);
    ui->comboBox_strokeMovement->setCurrentIndex(0);
    ui->comboBox_toolNumber->setCurrentIndex(0);
    ui->doubleSpinBox_diameterPulley->setValue(diameter_[0]);
}
