#include "ValveWindow.h"
#include "ui_ValveWindow.h"
#include "./Src/ValidatorFactory/ValidatorFactory.h"

ValveWindow::ValveWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ValveWindow)
{
    ui->setupUi(this);

    QValidator *validatorDigits = ValidatorFactory::create(ValidatorFactory::Type::Digits, this);
    QValidator *validatorDigitsDot = ValidatorFactory::create(ValidatorFactory::Type::DigitsDot, this);
    QValidator *noSpecialChars = ValidatorFactory::create(ValidatorFactory::Type::NoSpecialChars, this);

    ui->lineEdit_positionNumber->setValidator(noSpecialChars);
    ui->lineEdit_manufacturer->setValidator(noSpecialChars);
    ui->lineEdit_serialNumber->setValidator(noSpecialChars);
    ui->lineEdit_DN->setValidator(validatorDigits);
    ui->lineEdit_PN->setValidator(validatorDigits);
    ui->lineEdit_strokValve->setValidator(validatorDigitsDot);
    // ui->lineEdit_dinamicError->setValidator(noSpecialChars);
    ui->lineEdit_driveModel->setValidator(noSpecialChars);
    ui->lineEdit_driveRange->setValidator(validatorDigitsDot);
    ui->lineEdit_driveDiameter->setValidator(validatorDigitsDot);
    ui->lineEdit_diameterPulley->setValidator(validatorDigitsDot);

    ui->lineEdit_diameterPulley->setText(m_diameter[0]);

    connect(ui->comboBox_strokeMovement,
            &QComboBox::currentIndexChanged,
            this,
            &ValveWindow::StrokeChanged);

    connect(ui->comboBox_toolNumber,
            &QComboBox::currentIndexChanged,
            this,
            &ValveWindow::ToolChanged);

    connect(ui->lineEdit_driveDiameter,
            &QLineEdit::textChanged,
            this,
            &ValveWindow::DiameterChanged);

    connect(ui->pushButton, &QPushButton::clicked, this, &ValveWindow::ButtonClick);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &ValveWindow::Clear);

    DiameterChanged(ui->lineEdit_driveDiameter->text());

    connect(ui->comboBox_positionerType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &ValveWindow::onPositionerTypeChanged);

    onPositionerTypeChanged(ui->comboBox_positionerType->currentIndex());
}

ValveWindow::~ValveWindow()
{
    delete ui;
}

void ValveWindow::onPositionerTypeChanged(int index)
{
    const QString selected = ui->comboBox_positionerType->itemText(index);

    ui->comboBox_dinamicError->clear();

    if (selected == QStringLiteral("Интеллектуальный ЭПП")) {
        ui->comboBox_dinamicError->addItem(QStringLiteral("1,5"));
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }
    else if (selected == QStringLiteral("ЭПП") || selected == QStringLiteral("ПП")) {
        ui->comboBox_dinamicError->addItem(QStringLiteral("2,5"));
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }
}

void ValveWindow::SetRegistry(Registry *registry)
{
    m_registry = registry;
    m_valveInfo = m_registry->GetValveInfo();

    ui->comboBox_positionNumber->clear();
    ui->comboBox_positionNumber->addItems(m_registry->GetPositions());
    ui->comboBox_positionNumber->addItem(m_manualInput);

    QString lastPosition = m_registry->GetLastPosition();
    if (lastPosition == "") {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->count() - 1);
    } else {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->findText(lastPosition));
        PositionChanged(lastPosition);
    }

    connect(ui->comboBox_positionNumber,
        &QComboBox::currentTextChanged,
        this,
        &ValveWindow::PositionChanged);
}

void ValveWindow::SaveValveInfo()
{
    if (ui->comboBox_positionNumber->currentText() == m_manualInput)
        m_valveInfo = m_registry->GetValveInfo(ui->lineEdit_positionNumber->text());

    m_valveInfo->manufacturer = ui->lineEdit_manufacturer->text();
    m_valveInfo->valveModel = ui->lineEdit_valveModel->text();
    m_valveInfo->serialNumber = ui->lineEdit_serialNumber->text();

    m_valveInfo->DN = ui->lineEdit_DN->text();
    m_valveInfo->PN = ui->lineEdit_PN->text();

    m_valveInfo->positionerModel = ui->lineEdit_positionerModel->text();
    m_valveInfo->positionerType = ui->comboBox_positionerType->currentText();
    m_valveInfo->dinamicError = ui->comboBox_dinamicError->currentText().toDouble();

    m_valveInfo->solenoidValveModel = ui->lineEdit_solenoidValveModel->text();
    m_valveInfo->limitSwitchModel = ui->lineEdit_limitSwitchModel->text();
    m_valveInfo->positionSensorModel = ui->lineEdit_positionSensorModel->text();


    m_valveInfo->strokeMovement = ui->comboBox_strokeMovement->currentIndex();
    m_valveInfo->strokValve = ui->lineEdit_strokValve->text();
    m_valveInfo->driveModel = ui->lineEdit_driveModel->text();
    m_valveInfo->safePosition = ui->comboBox_safePosition->currentIndex();
    m_valveInfo->driveType = ui->comboBox_driveType->currentIndex();
    m_valveInfo->driveRange = ui->lineEdit_driveRange->text();
    m_valveInfo->driveDiameter = ui->lineEdit_driveDiameter->text().toDouble();
    m_valveInfo->toolNumber = ui->comboBox_toolNumber->currentIndex();
    // m_valveInfo->diameterPulley = ui->lineEdit_diameterPulley->text().toDouble();
    m_valveInfo->materialStuffingBoxSeal = ui->comboBox_materialStuffingBoxSeal->currentText();

    m_registry->SaveValveInfo();
}

void ValveWindow::PositionChanged(const QString &position)
{
    if (position == "Ручной ввод") {
        ui->lineEdit_positionNumber->setEnabled(true);
        return;
    }

    m_valveInfo = m_registry->GetValveInfo(position);

    ui->lineEdit_positionNumber->setText(position);
    ui->lineEdit_positionNumber->setEnabled(false);

    ui->lineEdit_manufacturer->setText(m_valveInfo->manufacturer);

    ui->lineEdit_valveModel->setText(m_valveInfo->valveModel);
    ui->lineEdit_serialNumber->setText(m_valveInfo->serialNumber);
    ui->lineEdit_DN->setText(m_valveInfo->DN);
    ui->lineEdit_PN->setText(m_valveInfo->PN);
    ui->lineEdit_strokValve->setText(m_valveInfo->strokValve);
    ui->lineEdit_positionerModel->setText(m_valveInfo->positionerModel);

    ui->lineEdit_solenoidValveModel->setText(m_valveInfo->solenoidValveModel);
    ui->lineEdit_limitSwitchModel->setText(m_valveInfo->limitSwitchModel);
    ui->lineEdit_positionSensorModel->setText(m_valveInfo->positionSensorModel);

    ui->lineEdit_positionNumber->setText(m_valveInfo->positionNumber);
    ui->lineEdit_driveModel->setText(m_valveInfo->driveModel);
    ui->lineEdit_driveRange->setText(m_valveInfo->driveRange);
    // ui->lineEdit_diameterPulley->setText(QString::number(m_valveInfo->diameterPulley));

    ui->lineEdit_driveDiameter->setText(QString::number(m_valveInfo->driveDiameter));

    ui->comboBox_safePosition->setCurrentIndex(m_valveInfo->safePosition);
    ui->comboBox_driveType->setCurrentIndex(m_valveInfo->driveType);
    ui->comboBox_strokeMovement->setCurrentIndex(m_valveInfo->strokeMovement);
    ui->comboBox_toolNumber->setCurrentIndex(m_valveInfo->toolNumber);
}

void ValveWindow::ButtonClick()
{
    if (ui->lineEdit_positionNumber->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите номер позиции");
        return;
    }

    if ((ui->lineEdit_manufacturer->text().isEmpty()) || (ui->lineEdit_valveModel->text().isEmpty())
        || (ui->lineEdit_serialNumber->text().isEmpty()) || (ui->lineEdit_DN->text().isEmpty())
        || (ui->lineEdit_PN->text().isEmpty()) || (ui->lineEdit_strokValve->text().isEmpty())
        || (ui->lineEdit_positionNumber->text().isEmpty()) || (ui->lineEdit_valveModel->text().isEmpty())
        || (ui->lineEdit_driveRange->text().isEmpty())) {

        QMessageBox::StandardButton button
            = QMessageBox::question(this,
                                    "Предупреждение",
                                    "Введены не все данные, вы действительно хотите продолжить?");

        if (button == QMessageBox::StandardButton::No) {
            return;
        }
    }

    OtherParameters *otherParameters = m_registry->GetOtherParameters();
    otherParameters->safePosition = ui->comboBox_safePosition->currentText();
    otherParameters->strokeMovement = ui->comboBox_strokeMovement->currentText();
    SaveValveInfo();

    accept();
}

void ValveWindow::StrokeChanged(quint16 n)
{
    ui->comboBox_toolNumber->setEnabled(n == 1);
    ui->lineEdit_diameterPulley->setEnabled(
        (n == 1)
        && (ui->comboBox_toolNumber->currentIndex() == ui->comboBox_toolNumber->count() - 1));
}

void ValveWindow::ToolChanged(quint16 n)
{
    if (ui->comboBox_toolNumber->currentText() == m_manualInput) {
        ui->lineEdit_diameterPulley->setEnabled(true);
    } else {
        ui->lineEdit_diameterPulley->setEnabled(false);
        ui->lineEdit_diameterPulley->setText(m_diameter[n]);
    }
}

void ValveWindow::DiameterChanged(const QString &text)
{
    double value = text.toDouble();
    ui->label_valueSquare->setText(QString().asprintf("%.2f", M_PI * value * value / 4));
}

void ValveWindow::Clear()
{
    ui->lineEdit_manufacturer->setText("");
    ui->lineEdit_valveModel->setText("");
    ui->lineEdit_serialNumber->setText("");
    ui->lineEdit_DN->setText("");
    ui->lineEdit_PN->setText("");
    ui->lineEdit_strokValve->setText("");
    ui->lineEdit_positionNumber->setText("");
    ui->lineEdit_valveModel->setText("");
    ui->lineEdit_driveRange->setText("");
    ui->lineEdit_driveDiameter->setText("");

    ui->lineEdit_diameterPulley->setText(m_diameter[0]);

    ui->comboBox_materialStuffingBoxSeal->setCurrentIndex(0);
    ui->comboBox_dinamicError->setCurrentIndex(0);
    ui->comboBox_safePosition->setCurrentIndex(0);
    ui->comboBox_driveType->setCurrentIndex(0);
    ui->comboBox_strokeMovement->setCurrentIndex(0);
    ui->comboBox_toolNumber->setCurrentIndex(0);
}
