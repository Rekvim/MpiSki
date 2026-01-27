#include "ValveWindow.h"
#include "ui_ValveWindow.h"
#include "./Src/ValidatorFactory/ValidatorFactory.h"
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
ValveWindow::ValveWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ValveWindow)
{
    ui->setupUi(this);

    QTimer::singleShot(0, this, [this]{
        auto *scr = screen(); // экран, где окно
        if (!scr) scr = QGuiApplication::primaryScreen();

        const QRect r = scr->availableGeometry();
        setWindowState(Qt::WindowNoState);
        setGeometry(r);
    });

    ui->tabWidget->setCurrentIndex(0);

    QValidator *validatorDigits = ValidatorFactory::create(ValidatorFactory::Type::Digits, this);
    QValidator *validatorDigitsDot = ValidatorFactory::create(ValidatorFactory::Type::DigitsDot, this);
    QValidator *noSpecialChars = ValidatorFactory::create(ValidatorFactory::Type::NoSpecialChars, this);

    ui->lineEdit_positionNumber->setValidator(noSpecialChars);
    ui->lineEdit_manufacturer->setValidator(noSpecialChars);
    ui->lineEdit_serialNumber->setValidator(noSpecialChars);
    ui->lineEdit_DN->setValidator(validatorDigits);
    ui->lineEdit_PN->setValidator(validatorDigits);
    ui->lineEdit_strokValve->setValidator(validatorDigitsDot);
    ui->lineEdit_driveModel->setValidator(noSpecialChars);
    // ui->lineEdit_driveRange->setValidator(validatorDigitsDot);
    ui->lineEdit_driveDiameter->setValidator(validatorDigitsDot);
    ui->lineEdit_pulleyDiameter->setValidator(validatorDigitsDot);

    connect(ui->lineEdit_pulleyDiameter, &QLineEdit::textChanged,
            this, &ValveWindow::diameterChanged);

    connect(ui->comboBox_strokeMovement, &QComboBox::currentIndexChanged,
            this, &ValveWindow::strokeChanged);

    connect(ui->comboBox_toolNumber, &QComboBox::currentIndexChanged,
            this, &ValveWindow::toolChanged);

    connect(ui->lineEdit_driveDiameter, &QLineEdit::textChanged,
            this, &ValveWindow::diameterChanged);

    connect(ui->comboBox_positionerType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ValveWindow::onPositionerTypeChanged);

    onPositionerTypeChanged(ui->comboBox_positionerType->currentIndex());

    ui->lineEdit_pulleyDiameter->setText(m_diameter[0]);
    diameterChanged(m_diameter[0]);
}

void ValveWindow::setPatternType(SelectTests::PatternType pattern)
{
    m_patternType = pattern;
    applyPatternVisibility();
}

void ValveWindow::applyPatternVisibility()
{
    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT:
        ui->widget_positionSensorModel->setVisible(false);
        break;
    case SelectTests::Pattern_C_CVT:
        ui->widget_positionSensorModel->setVisible(false);
        ui->widget_solenoidValveModel->setVisible(false);
        ui->widget_limitSwitchModel->setVisible(false);
        break;
    case SelectTests::Pattern_B_SACVT:
        break;
    case SelectTests::Pattern_C_SACVT:
        break;
    case SelectTests::Pattern_C_SOVT:
        ui->widget_positionerModel->setVisible(false);
        ui->widget_dinamicError_positionerType->setVisible(false);

        break;
    default:
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбран корректный паттерн!"));
        return;
    }
}

void ValveWindow::onPositionerTypeChanged(quint8 index)
{
    const QString selected = ui->comboBox_positionerType->itemText(index);

    ui->comboBox_dinamicError->clear();

    if (selected == tr("Интеллектуальный ЭПП")) {
        ui->comboBox_dinamicError->addItem(QStringLiteral("1.5"));
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }
    else if (selected == tr("ЭПП") || selected == tr("ПП")) {
        ui->comboBox_dinamicError->addItem(QStringLiteral("2.5"));
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }
}

void ValveWindow::setRegistry(Registry *registry)
{
    m_registry = registry;
    m_valveInfo = m_registry->getValveInfo();

    ui->comboBox_positionNumber->clear();
    ui->comboBox_positionNumber->addItems(m_registry->getPositions());
    ui->comboBox_positionNumber->addItem(kManualInput);

    QString lastPosition = m_registry->getLastPosition();
    if (lastPosition == "") {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->count() - 1);
    } else {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->findText(lastPosition));
        positionChanged(lastPosition);
    }

    connect(ui->comboBox_positionNumber,
        &QComboBox::currentTextChanged,
        this,
        &ValveWindow::positionChanged);
}

void ValveWindow::saveValveInfo()
{
    if (ui->comboBox_positionNumber->currentText() == kManualInput)
        m_valveInfo = m_registry->getValveInfo(ui->lineEdit_positionNumber->text());

    m_valveInfo->manufacturer = ui->lineEdit_manufacturer->text();
    m_valveInfo->valveModel = ui->lineEdit_valveModel->text();
    m_valveInfo->serialNumber = ui->lineEdit_serialNumber->text();

    m_valveInfo->DN = ui->lineEdit_DN->text();
    m_valveInfo->PN = ui->lineEdit_PN->text();

    m_valveInfo->positionerModel = ui->lineEdit_positionerModel->text();

    m_valveInfo->positionerType = m_patternType == SelectTests::Pattern_C_SOVT
        ? ui->comboBox_positionerType->currentText()
        : "";

    m_valveInfo->dinamicErrorRecomend = ui->comboBox_dinamicError->currentText().toDouble();

    m_valveInfo->solenoidValveModel = ui->lineEdit_solenoidValveModel->text();
    m_valveInfo->limitSwitchModel = ui->lineEdit_limitSwitchModel->text();
    m_valveInfo->positionSensorModel = ui->lineEdit_positionSensorModel->text();

    m_valveInfo->strokeMovement = ui->comboBox_strokeMovement->currentIndex();
    m_valveInfo->strokValve = ui->lineEdit_strokValve->text();
    m_valveInfo->driveModel = ui->lineEdit_driveModel->text();
    m_valveInfo->safePosition = ui->comboBox_safePosition->currentIndex();
    m_valveInfo->driveType = ui->comboBox_driveType->currentIndex();
    m_valveInfo->driveRecomendRange = ui->lineEdit_driveRange->text();
    m_valveInfo->driveDiameter = ui->lineEdit_driveDiameter->text().toDouble();
    m_valveInfo->toolNumber = ui->comboBox_toolNumber->currentIndex();
    m_valveInfo->diameterPulley = ui->lineEdit_pulleyDiameter->text().toDouble();
    m_valveInfo->materialStuffingBoxSeal = ui->comboBox_materialStuffingBoxSeal->currentText();

    m_valveInfo->crossingLimits.frictionCoefLowerLimit =
        ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->text().toDouble();
    m_valveInfo->crossingLimits.frictionCoefUpperLimit =
        ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->text().toDouble();

    // Линейная характеристика — один нижний порог
    m_valveInfo->crossingLimits.linearCharacteristicLowerLimit =
        ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->text().toDouble();

    // Диапазон хода — один верхний порог (берём из твоего range_lowerLimit)
    m_valveInfo->crossingLimits.rangeUpperLimit =
        ui->lineEdit_crossingLimits_range_lowerLimit->text().toDouble();

    // Пружина
    m_valveInfo->crossingLimits.springLowerLimit =
        ui->lineEdit_crossingLimits_spring_lowerLimit->text().toDouble();
    m_valveInfo->crossingLimits.springUpperLimit =
        ui->lineEdit_crossingLimits_spring_upperLimit->text().toDouble();

    m_valveInfo->crossingLimits.frictionEnabled =
        ui->checkBox_crossingLimits_coefficientFriction->isChecked();

    m_valveInfo->crossingLimits.linearCharacteristicEnabled =
        ui->checkBox_crossingLimits_linearCharacteristic->isChecked();

    m_valveInfo->crossingLimits.rangeEnabled =
        ui->checkBox_crossingLimits_range->isChecked();

    m_valveInfo->crossingLimits.springEnabled =
        ui->checkBox_crossingLimits_spring->isChecked();

    m_valveInfo->crossingLimits.dynamicErrorEnabled =
        ui->checkBox_crossingLimits_dinamicError->isChecked();

    m_registry->saveValveInfo();
}

void ValveWindow::positionChanged(const QString &position)
{
    if (position == kManualInput) {
        ui->lineEdit_positionNumber->setEnabled(true);
        return;
    }

    m_valveInfo = m_registry->getValveInfo(position);

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
    ui->lineEdit_driveRange->setText(m_valveInfo->driveRecomendRange);
    ui->lineEdit_pulleyDiameter->setText(QString::number(m_valveInfo->diameterPulley));

    ui->lineEdit_driveDiameter->setText(QString::number(m_valveInfo->driveDiameter));

    ui->comboBox_safePosition->setCurrentIndex(m_valveInfo->safePosition);
    ui->comboBox_driveType->setCurrentIndex(m_valveInfo->driveType);
    ui->comboBox_strokeMovement->setCurrentIndex(m_valveInfo->strokeMovement);
    ui->comboBox_toolNumber->setCurrentIndex(m_valveInfo->toolNumber);


    // --- enable-флаги -> checkBox-ы ---

    ui->checkBox_crossingLimits_coefficientFriction->setChecked(
        m_valveInfo->crossingLimits.frictionEnabled);

    ui->checkBox_crossingLimits_linearCharacteristic->setChecked(
        m_valveInfo->crossingLimits.linearCharacteristicEnabled);

    ui->checkBox_crossingLimits_range->setChecked(
        m_valveInfo->crossingLimits.rangeEnabled);

    ui->checkBox_crossingLimits_spring->setChecked(
        m_valveInfo->crossingLimits.springEnabled);

    ui->checkBox_crossingLimits_dinamicError->setChecked(
        m_valveInfo->crossingLimits.dynamicErrorEnabled);

    // Коэффициент трения (нижний / верхний)
    ui->lineEdit_crossingLimits_coefficientFriction_lowerLimit->setText(
        QString::number(m_valveInfo->crossingLimits.frictionCoefLowerLimit, 'f', 2));
    ui->lineEdit_crossingLimits_coefficientFriction_upperLimit->setText(
        QString::number(m_valveInfo->crossingLimits.frictionCoefUpperLimit, 'f', 2));

    // Линейная характеристика — один порог (нижний)
    ui->lineEdit_crossingLimits_linearCharacteristic_lowerLimit->setText(
        QString::number(m_valveInfo->crossingLimits.linearCharacteristicLowerLimit, 'f', 2));

    // Диапазон хода — один порог (верхний); тут у тебя lineEdit_*_range_lowerLimit,
    // но по структуре это "верхний". Если захочешь — потом переименуешь в ui.
    ui->lineEdit_crossingLimits_range_lowerLimit->setText(
        QString::number(m_valveInfo->crossingLimits.rangeUpperLimit, 'f', 2));

    // Пружина — нижний и верхний
    ui->lineEdit_crossingLimits_spring_lowerLimit->setText(
        QString::number(m_valveInfo->crossingLimits.springLowerLimit, 'f', 2));
    ui->lineEdit_crossingLimits_spring_upperLimit->setText(
        QString::number(m_valveInfo->crossingLimits.springUpperLimit, 'f', 2));

    // // Динамическая ошибка: от 0 до dinamicErrorRecomend
    // ui->lineEdit_crossingLimits_dynamicError_upperLimit->setText(
    //     QString::number(m_valveInfo->dinamicErrorRecomend, 'f', 2));
}

void ValveWindow::strokeChanged(quint16 n)
{
    ui->comboBox_toolNumber->setEnabled(n == 1);
    ui->lineEdit_pulleyDiameter->setEnabled(
        (n == 1)
        && (ui->comboBox_toolNumber->currentIndex() == ui->comboBox_toolNumber->count() - 1));
}

void ValveWindow::toolChanged(quint16 n)
{
    if (ui->comboBox_toolNumber->currentText() == kManualInput) {
        ui->lineEdit_pulleyDiameter->setEnabled(true);
    } else {
        ui->lineEdit_pulleyDiameter->setEnabled(false);
        ui->lineEdit_pulleyDiameter->setText(m_diameter[n]);
    }
}

void ValveWindow::diameterChanged(const QString &text)
{
    double value = text.toDouble();
    ui->label_valueSquare->setText(QString().asprintf("%.2f", M_PI * value * value / 4));
}

void ValveWindow::on_pushButton_netWindow_clicked()
{
    if (ui->lineEdit_positionNumber->text().isEmpty()) {
        QMessageBox::warning(this, 
            tr("Ошибка"),
            tr("Введите номер позиции"));
        return;
    }

    if ((ui->lineEdit_manufacturer->text().isEmpty()) || (ui->lineEdit_valveModel->text().isEmpty())
        || (ui->lineEdit_serialNumber->text().isEmpty()) || (ui->lineEdit_DN->text().isEmpty())
        || (ui->lineEdit_PN->text().isEmpty()) || (ui->lineEdit_strokValve->text().isEmpty())
        || (ui->lineEdit_positionNumber->text().isEmpty()) || (ui->lineEdit_valveModel->text().isEmpty())
        || (ui->lineEdit_driveRange->text().isEmpty())) {

        QMessageBox::StandardButton button
            = QMessageBox::question(this,
                                    tr("Предупреждение"),
                                    tr("Введены не все данные, вы действительно хотите продолжить?"));

        if (button == QMessageBox::StandardButton::No) {
            return;
        }
    }

    OtherParameters *otherParameters = m_registry->getOtherParameters();
    otherParameters->safePosition = ui->comboBox_safePosition->currentText();
    otherParameters->strokeMovement = ui->comboBox_strokeMovement->currentText();
    saveValveInfo();

    accept();
}

void ValveWindow::on_pushButton_clear_clicked()
{
    ui->lineEdit_manufacturer->clear();
    ui->lineEdit_valveModel->clear();
    ui->lineEdit_serialNumber->clear();
    ui->lineEdit_DN->clear();
    ui->lineEdit_PN->clear();
    ui->lineEdit_strokValve->clear();
    ui->lineEdit_positionNumber->clear();
    ui->lineEdit_valveModel->clear();
    ui->lineEdit_driveRange->clear();
    ui->lineEdit_driveDiameter->clear();

    ui->lineEdit_pulleyDiameter->setText(m_diameter[0]);

    ui->comboBox_materialStuffingBoxSeal->setCurrentIndex(0);
    ui->comboBox_dinamicError->setCurrentIndex(0);
    ui->comboBox_safePosition->setCurrentIndex(0);
    ui->comboBox_driveType->setCurrentIndex(0);
    ui->comboBox_strokeMovement->setCurrentIndex(0);
    ui->comboBox_toolNumber->setCurrentIndex(0);
}

