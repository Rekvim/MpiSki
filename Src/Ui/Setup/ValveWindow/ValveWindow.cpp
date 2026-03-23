#include "ValveWindow.h"
#include "Mapper.h"
#include "Logic.h"

#include "ui_ValveWindow.h"
#include "./Src/ValidatorFactory/ValidatorFactory.h"
#include "Src/Utils/Shortcuts/TabBinder.h"

#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QShortcut>
#include <QColorDialog>

ValveWindow::ValveWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ValveWindow)
{
    ui->setupUi(this);

    setupWindowGeometry();
    setupTabs();
    setupValidators();
    setupConnections();
    setupInitialUi();
}

void ValveWindow::updateManufacturerUi()
{
    const bool manual =
        (ui->comboBox_manufacturer->currentText() == kManualInput);

    ui->lineEdit_manufacturer->setEnabled(manual);

    if (!manual)
        ui->lineEdit_manufacturer->clear();
}

void ValveWindow::setupConnections()
{
    connect(ui->comboBox_manufacturer, &QComboBox::currentTextChanged,
            this, &ValveWindow::updateManufacturerUi);

    connect(ui->comboBox_strokeMovement, &QComboBox::currentIndexChanged,
            this, &ValveWindow::strokeChanged);

    connect(ui->comboBox_toolNumber, &QComboBox::currentIndexChanged,
            this, &ValveWindow::toolChanged);

    connect(ui->lineEdit_driveDiameter, &QLineEdit::textChanged,
            this, [this](const QString& text) {
        ui->label_valueSquare->setText(
            QString::number(Logic::calculateCircleArea(text.toDouble()), 'f', 2)
            );
    });

    connect(ui->comboBox_positionerType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ValveWindow::onPositionerTypeChanged);

    connect(ui->comboBox_driveType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ValveWindow::onDriveTypeChanged);

    connect(ui->comboBox_materialStuffingBoxSeal, &QComboBox::currentTextChanged,
            this, &ValveWindow::applyFrictionLimitsFromStuffingBoxSeal);
}

void ValveWindow::setupInitialUi()
{
    updateManufacturerUi();

    onDriveTypeChanged(ui->comboBox_driveType->currentIndex());
    onPositionerTypeChanged(ui->comboBox_positionerType->currentIndex());

    ui->lineEdit_pulleyDiameter->setText(m_diameter[0]);
}

void ValveWindow::setupWindowGeometry()
{
    QTimer::singleShot(0, this, [this]{
        auto *scr = screen();
        if (!scr)
            scr = QGuiApplication::primaryScreen();

        const QRect r = scr->availableGeometry();
        setWindowState(Qt::WindowNoState);
        setGeometry(r);
    });
}

void ValveWindow::bindTabShortcut(int key, QWidget* tab)
{
    auto* sc = new QShortcut(QKeySequence(QString::number(key)), this);
    sc->setContext(Qt::ApplicationShortcut);

    connect(sc, &QShortcut::activated, this, [this, tab] {
        ui->tabWidget->setCurrentWidget(tab);
    });
}

void ValveWindow::setupTabs()
{
    TabBinder::bindNumbers(this, ui->tabWidget);
}

void ValveWindow::setupValidators()
{
    QValidator *validatorDigits =
        ValidatorFactory::create(ValidatorFactory::Type::Digits, this);

    QValidator *validatorDigitsDot =
        ValidatorFactory::create(ValidatorFactory::Type::DigitsDot, this);

    QValidator *noSpecialChars =
        ValidatorFactory::create(ValidatorFactory::Type::NoSpecialChars, this);

    ui->lineEdit_positionNumber->setValidator(noSpecialChars);
    ui->lineEdit_manufacturer->setValidator(noSpecialChars);
    ui->lineEdit_serialNumber->setValidator(noSpecialChars);
    ui->lineEdit_DN->setValidator(validatorDigits);
    ui->lineEdit_PN->setValidator(validatorDigits);
    ui->lineEdit_valveStroke->setValidator(validatorDigitsDot);
    ui->lineEdit_driveModel->setValidator(noSpecialChars);
    ui->lineEdit_driveDiameter->setValidator(validatorDigitsDot);
    ui->lineEdit_pulleyDiameter->setValidator(validatorDigitsDot);

    ui->lineEdit_linearMin->setValidator(
        ValidatorFactory::create(ValidatorFactory::Type::DigitsDot, this));

    ui->lineEdit_linearMax->setValidator(
        ValidatorFactory::create(ValidatorFactory::Type::DigitsDot, this));
}

void ValveWindow::applyFrictionLimitsFromStuffingBoxSeal()
{
    const StuffingBoxSeal seal =
        ValveEnums::stuffingBoxSealFromCombo(ui->comboBox_materialStuffingBoxSeal);

    auto limits = Logic::frictionLimitsForSeal(seal);
    if (!limits.has_value()) {
        return;
    }

    ui->lineEdit_crossingLimits_coefficientFriction_lower
        ->setText(QString::number(limits->lower, 'f', 2));

    ui->lineEdit_crossingLimits_coefficientFriction_upper
        ->setText(QString::number(limits->upper, 'f', 2));
}

void ValveWindow::loadLinearRange()
{
    auto [min, max] = m_settings.linearRange();

    ui->lineEdit_linearMin->setText(QString::number(min));
    ui->lineEdit_linearMax->setText(QString::number(max));
}

void ValveWindow::saveLinearRange()
{
    double min = ui->lineEdit_linearMin->text().toDouble();
    double max = ui->lineEdit_linearMax->text().toDouble();

    m_settings.setLinearRange(min, max);
}

void ValveWindow::setPatternType(SelectTests::PatternType pattern)
{
    m_patternType = pattern;
    applyPatternVisibility();
}

void ValveWindow::onDriveTypeChanged(int)
{
    const DriveType type = ValveEnums::driveTypeFromCombo(ui->comboBox_driveType);
    const Logic::DriveTypeRule rule = Logic::driveTypeRule(type);

    ui->lineEdit_driveRange->setEnabled(rule.driveRangeEnabled);

    if (!rule.driveRangeEnabled) {
        ui->lineEdit_driveRange->setText(rule.driveRangeText);
        return;
    }

    if (ui->lineEdit_driveRange->text().trimmed() == QStringLiteral("Привод ДД")) {
        ui->lineEdit_driveRange->clear();
    }
}

void ValveWindow::applyPatternVisibility()
{
    switch (m_patternType) {
    case SelectTests::Pattern_B_CVT:
        ui->widget_positionSensorModel->setVisible(false);
        ui->widget_solenoidValveModel->setVisible(false);
        ui->widget_limitSwitchModel->setVisible(false);
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

void ValveWindow::onPositionerTypeChanged(quint8)
{
    const PositionerType type = ValveEnums::positionerTypeFromCombo(ui->comboBox_positionerType);
    const Logic::DynamicErrorRule rule = Logic::dynamicErrorRule(type);

    ui->comboBox_dinamicError->clear();
    ui->comboBox_dinamicError->addItems(rule.values);

    if (!rule.values.isEmpty()) {
        ui->comboBox_dinamicError->setCurrentIndex(0);
    }

    ui->checkBox_crossingLimits_dinamicError->setEnabled(rule.crossingLimitsEnabled);

    if (rule.forceUncheckCrossingLimits) {
        ui->checkBox_crossingLimits_dinamicError->setChecked(false);
    }
}

void ValveWindow::setRegistry(Registry *registry)
{
    m_registry = registry;

    m_registry->loadObjectInfo();
    loadLinearRange();

    ui->comboBox_positionNumber->clear();
    ui->comboBox_positionNumber->addItems(m_registry->positions());
    ui->comboBox_positionNumber->addItem(kManualInput);

    const QString last = m_registry->lastPosition();
    if (!last.isEmpty() && m_registry->loadValveInfo(last)) {
        m_local = m_registry->valveInfo();
        Mapper::write(*this, m_local);
    }

    if (last == "") {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->count() - 1);
    } else {
        ui->comboBox_positionNumber->setCurrentIndex(ui->comboBox_positionNumber->findText(last));
        positionChanged(last);
    }

    connect(ui->comboBox_positionNumber, &QComboBox::currentTextChanged,
        this, &ValveWindow::positionChanged);
}

void ValveWindow::saveValveInfo()
{
    m_local = Mapper::read(*this);
    m_registry->valveInfo() = m_local;
    m_registry->saveValveInfo();
}

void ValveWindow::positionChanged(const QString &position)
{
    if (position == kManualInput) {
        ui->lineEdit_positionNumber->clear();
        ui->lineEdit_positionNumber->setEnabled(true);
        return;
    }

    if (m_registry->loadValveInfo(position)) {
        m_local = m_registry->valveInfo();
        Mapper::write(*this, m_local);
    }
}

void ValveWindow::strokeChanged(quint16 n)
{
    ui->comboBox_toolNumber->setEnabled(n == 1);
    ui->lineEdit_pulleyDiameter->setEnabled(
        (n == 1) &&
        (ui->comboBox_toolNumber->currentIndex() == ui->comboBox_toolNumber->count() - 1));
}

void ValveWindow::toolChanged(quint16 n)
{
    const bool manual = (ui->comboBox_toolNumber->currentText() == kManualInput);

    ui->lineEdit_pulleyDiameter->setEnabled(manual);

    if (manual)
        return;

    ui->lineEdit_pulleyDiameter->setText(m_diameter[n]);
}

void ValveWindow::on_pushButton_netWindow_clicked()
{
    saveLinearRange();

    double min = ui->lineEdit_linearMin->text().toDouble();
    double max = ui->lineEdit_linearMax->text().toDouble();

    if (min >= max) {
        QMessageBox::warning(this,
                             tr("Ошибка"),
                             tr("Минимальное значение должно быть меньше максимального"));
        return;
    }

    if (ui->lineEdit_positionNumber->text().isEmpty()) {
        QMessageBox::warning(this, 
            tr("Ошибка"),
            tr("Введите номер позиции"));
        return;
    }

    const QString manuf =
        (ui->comboBox_manufacturer->currentText() == kManualInput)
            ? ui->lineEdit_manufacturer->text()
            : ui->comboBox_manufacturer->currentText();

    if (manuf.isEmpty() || ui->lineEdit_valveModel->text().isEmpty()
        || ui->lineEdit_serialNumber->text().isEmpty()
        || ui->lineEdit_DN->text().isEmpty()
        || ui->lineEdit_PN->text().isEmpty()
        || ui->lineEdit_valveStroke->text().isEmpty()
        || ui->lineEdit_driveRange->text().isEmpty()) {

        QMessageBox::StandardButton button
            = QMessageBox::question(this,
                                    tr("Предупреждение"),
                                    tr("Введены не все данные, вы действительно хотите продолжить?"));

        if (button == QMessageBox::StandardButton::No) {
            return;
        }
    }

    auto& other = m_registry->otherParameters();

    other.safePosition = Logic::toString(ValveEnums::safePositionFromCombo(ui->comboBox_safePosition));
    other.strokeMovement = Logic::toString(ValveEnums::strokeMovementFromCombo(ui->comboBox_strokeMovement));

    saveValveInfo();
    accept();
}

void ValveWindow::on_pushButton_clear_clicked()
{
    for (auto edit : findChildren<QLineEdit*>())
        edit->clear();

    for (auto edit : findChildren<QComboBox*>())
        edit->setCurrentIndex(0);

    ui->lineEdit_pulleyDiameter->setText(m_diameter[0]);
}

