// CyclicTestSettings.cpp
#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"
#include "./Src/ValidatorFactory/RegexPatterns.h"

CyclicTestSettings::CyclicTestSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

    ui->verticalLayout_shutOff->parentWidget()->hide();

    // Регулирующий
    connect(ui->pushButton_addRangeRegulatory, &QPushButton::clicked, this, &CyclicTestSettings::onAddValueClicked);
    connect(ui->pushButton_editRangeRegulatory, &QPushButton::clicked, this, &CyclicTestSettings::onEditValueClicked);
    connect(ui->pushButton_removeRangeRegulatory, &QPushButton::clicked, this, &CyclicTestSettings::onRemoveValueClicked);
    connect(ui->pushButton_addDelayRegulatory, &QPushButton::clicked, this, &CyclicTestSettings::onAddDelayClicked);
    connect(ui->pushButton_editDelayRegulatory, &QPushButton::clicked, this, &CyclicTestSettings::onEditDelayClicked);
    connect(ui->pushButton_removeDelayRegulatory, &QPushButton::clicked, this, &CyclicTestSettings::onRemoveDelayClicked);

    // Отсечной
    connect(ui->pushButton_addDelayShutOff,    &QPushButton::clicked, this, &CyclicTestSettings::onAddDelayShutOffClicked);
    connect(ui->pushButton_editDelayShutOff,   &QPushButton::clicked, this, &CyclicTestSettings::onEditDelayShutOffClicked);
    connect(ui->pushButton_removeDelayShutOff, &QPushButton::clicked, this, &CyclicTestSettings::onRemoveDelayShutOffClicked);

    // Общее
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButton_start,  &QPushButton::clicked, this, &CyclicTestSettings::onPushButtonStartClicked);

    connect(ui->comboBox_testSelection, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CyclicTestSettings::onTestSelectionChanged);

    onTestSelectionChanged();
}


CyclicTestSettings::~CyclicTestSettings()
{
    delete ui;
}

static void setLayoutVisible(QLayout* layout, bool visible) {
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem* item = layout->itemAt(i);
        if (QWidget* w = item->widget()) {
            w->setVisible(visible);
        }
        if (QLayout* sub = item->layout()) {
            setLayoutVisible(sub, visible);
        }
    }
}

void CyclicTestSettings::onTestSelectionChanged()
{
    const QString sel = ui->comboBox_testSelection->currentText();
    bool showReg = (sel == "Регулирующий" || sel == "Запорно-регулирующий");
    bool showOff = (sel == "Отсечной"    || sel == "Запорно-регулирующий");
    setLayoutVisible(ui->verticalLayout_regulatory, showReg);
    setLayoutVisible(ui->verticalLayout_shutOff,    showOff);
}

// --- Регулирующий

void CyclicTestSettings::onAddValueClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Добавить последовательность",
                                         "Введите значения (через «-»):",
                                         QLineEdit::Normal, "", &ok);
    if (ok && !text.trimmed().isEmpty()) {
        ui->listWidget_testRangeRegulatory->addItem(text.trimmed());
        ui->listWidget_testRangeRegulatory->setCurrentRow(
            ui->listWidget_testRangeRegulatory->count()-1);
    }
}

void CyclicTestSettings::onEditValueClicked()
{
    auto *item = ui->listWidget_testRangeRegulatory->currentItem();
    if (!item) return;
    bool ok;
    QString txt = QInputDialog::getText(this, "Изменить последовательность",
                                        "Новое значение:", QLineEdit::Normal,
                                        item->text(), &ok);
    if (ok && !txt.trimmed().isEmpty()) {
        item->setText(txt.trimmed());
    }
}

void CyclicTestSettings::onRemoveValueClicked()
{
    delete ui->listWidget_testRangeRegulatory->currentItem();
}

void CyclicTestSettings::onAddDelayClicked()
{
    bool ok;
    int v = QInputDialog::getInt(this, "Добавить задержку",
                                 "Введите значение в секундах:", 10, 1, 3600, 1, &ok);
    if (ok) {
        ui->listWidget_delayTimeRegulatory->addItem(QString::number(v));
        ui->listWidget_delayTimeRegulatory->setCurrentRow(
            ui->listWidget_delayTimeRegulatory->count()-1);
    }
}

void CyclicTestSettings::onEditDelayClicked()
{
    auto *item = ui->listWidget_delayTimeRegulatory->currentItem();
    if (!item) return;
    bool ok;
    int v = QInputDialog::getInt(this, "Изменить задержку",
                                 "Новое значение (сек):",
                                 item->text().toInt(), 1, 3600, 1, &ok);
    if (ok) {
        item->setText(QString::number(v));
    }
}

void CyclicTestSettings::onRemoveDelayClicked()
{
    delete ui->listWidget_delayTimeRegulatory->currentItem();
}

// --- Отсечной

void CyclicTestSettings::onAddDelayShutOffClicked()
{
    bool ok;
    int v = QInputDialog::getInt(this, "Добавить задержку (Отсечной)",
                                 "Введите значение в секундах:", 10, 1, 3600, 1, &ok);
    if (ok) {
        ui->listWidget_delayTimeShutOff->addItem(QString::number(v));
        ui->listWidget_delayTimeShutOff->setCurrentRow(
            ui->listWidget_delayTimeShutOff->count()-1);
    }
}

void CyclicTestSettings::onEditDelayShutOffClicked()
{
    auto *item = ui->listWidget_delayTimeShutOff->currentItem();
    if (!item) return;
    bool ok;
    int v = QInputDialog::getInt(this, "Изменить задержку (Отсечной)",
                                 "Новое значение (сек):",
                                 item->text().toInt(), 1, 3600, 1, &ok);
    if (ok) {
        item->setText(QString::number(v));
    }
}

void CyclicTestSettings::onRemoveDelayShutOffClicked()
{
    delete ui->listWidget_delayTimeShutOff->currentItem();
}

// --- Общее

void CyclicTestSettings::onPushButtonStartClicked()
{
    // Регулирующий
    auto *seqReg = ui->listWidget_testRangeRegulatory->currentItem();
    if (!seqReg) {
        QMessageBox::warning(this, "Ошибка",
                             "Выберите или добавьте хотя бы одну последовательность (регулирующий).");
        return;
    }
    QString sReg = seqReg->text().trimmed();
    auto m1 = RegexPatterns::digitsHyphens().match(sReg);
    if (!m1.hasMatch() || m1.capturedLength() != sReg.length()) {
        QMessageBox::warning(this, "Ошибка",
                             "Регулирующая последовательность: только цифры и дефисы.");
        return;
    }
    m_parameters.sequence = sReg;

    auto *delReg = ui->listWidget_delayTimeRegulatory->currentItem();
    if (!delReg) {
        QMessageBox::warning(this, "Ошибка",
                             "Выберите время задержки (регулирующий).");
        return;
    }
    QString dReg = delReg->text().trimmed();
    auto m2 = RegexPatterns::digits().match(dReg);
    if (!m2.hasMatch() || m2.capturedLength() != dReg.length()) {
        QMessageBox::warning(this, "Ошибка",
                             "Задержка (регулирующий): положительное число.");
        return;
    }
    m_parameters.delay_sec = dReg.toInt();

    QTime holdReg = ui->timeEdit_retentionTimeRegulatory->time();
    int hRegSec = holdReg.minute()*60 + holdReg.second();
    if (hRegSec <= 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Время удержания (регулирующий) > 00:00.");
        return;
    }
    m_parameters.hold_time_sec = hRegSec;

    QString cReg = ui->lineEdit_numberCyclesRegulatory->text().trimmed();
    auto m3 = RegexPatterns::digits().match(cReg);
    if (!m3.hasMatch() || m3.capturedLength() != cReg.length() || cReg.isEmpty()) {
        QMessageBox::warning(this, "Ошибка",
                             "Циклы (регулирующий): введите положительное целое.");
        return;
    }
    m_parameters.num_cycles = cReg.toInt();
    if (m_parameters.num_cycles <= 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Число циклов (регулирующий) > 0.");
        return;
    }

    // Отсечной
    auto *seqOff = ui->listWidget_testRangeShutOff->currentItem();
    if (!seqOff) {
        QMessageBox::warning(this, "Ошибка",
                             "Выберите хотя бы одну последовательность (отсечной).");
        return;
    }
    QString sOff = seqOff->text().trimmed();
    auto m4 = RegexPatterns::digitsHyphens().match(sOff);
    if (!m4.hasMatch() || m4.capturedLength() != sOff.length()) {
        QMessageBox::warning(this, "Ошибка",
                             "Отсечной: только цифры и дефисы.");
        return;
    }
    m_parameters.shutoff_sequence = sOff;

    auto *delOff = ui->listWidget_delayTimeShutOff->currentItem();
    if (!delOff) {
        QMessageBox::warning(this, "Ошибка",
                             "Выберите время задержки (отсечной).");
        return;
    }
    QString dOff = delOff->text().trimmed();
    auto m5 = RegexPatterns::digits().match(dOff);
    if (!m5.hasMatch() || m5.capturedLength() != dOff.length()) {
        QMessageBox::warning(this, "Ошибка",
                             "Задержка (отсечной): положительное число.");
        return;
    }
    m_parameters.shutoff_delay_sec = dOff.toInt();

    QTime holdOff = ui->timeEdit_retentionTimeShutOff->time();
    int hOffSec = holdOff.minute()*60 + holdOff.second();
    if (hOffSec <= 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Время удержания (отсечной) > 00:00.");
        return;
    }
    m_parameters.shutoff_hold_time_sec = hOffSec;

    QString cOff = ui->lineEdit_numberCyclesShutOff->text().trimmed();
    auto m6 = RegexPatterns::digits().match(cOff);
    if (!m6.hasMatch() || m6.capturedLength() != cOff.length() || cOff.isEmpty()) {
        QMessageBox::warning(this, "Ошибка",
                             "Циклы (отсечной): введите положительное целое.");
        return;
    }
    m_parameters.shutoff_num_cycles = cOff.toInt();
    if (m_parameters.shutoff_num_cycles <= 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Число циклов (отсечной) > 0.");
        return;
    }

    m_parameters.shutoff_enable_20mA = ui->checkBox_2mA_enable->isChecked();

    m_parameters.shutoff_DO[0] = ui->pushButton_DO0_ShutOff->isChecked();
    m_parameters.shutoff_DO[1] = ui->pushButton_DO1_ShutOff->isChecked();
    m_parameters.shutoff_DO[2] = ui->pushButton_DO2_ShutOff->isChecked();
    m_parameters.shutoff_DO[3] = ui->pushButton_DO3_ShutOff->isChecked();

    m_parameters.shutoff_DI[0] = ui->checkBox_DI1_ShutOff->isChecked();
    m_parameters.shutoff_DI[1] = ui->checkBox_DI2_ShutOff->isChecked();

    accept();
}
