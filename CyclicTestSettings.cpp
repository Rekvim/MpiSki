#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"
#include "./Src/ValidatorFactory/RegexPatterns.h"

CyclicTestSettings::CyclicTestSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

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

    // Общие
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

void CyclicTestSettings::onTestSelectionChanged()
{
    const QString sel = ui->comboBox_testSelection->currentText();
    if (sel == "Регулирующий") {
        m_parameters.testType = TestParameters::Regulatory;
        ui->widget_retentionTimeRegulatory->setVisible(true);
        ui->widget_shutOff->setVisible(false);
    }
    else if (sel == "Отсечной") {
        m_parameters.testType = TestParameters::Shutoff;
        ui->widget_retentionTimeRegulatory->setVisible(false);
        ui->widget_shutOff->setVisible(true);
    }
    else {
        m_parameters.testType = TestParameters::Combined;
        ui->widget_retentionTimeRegulatory->setVisible(true);
        ui->widget_shutOff->setVisible(true);
    }
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
        ui->listWidget_testRangeRegulatory->setCurrentRow(ui->listWidget_testRangeRegulatory->count() - 1);
    }
}

void CyclicTestSettings::onEditValueClicked()
{
    if (auto *item = ui->listWidget_testRangeRegulatory->currentItem()) {
        bool ok;
        QString txt = QInputDialog::getText(this, "Изменить последовательность",
                                            "Новое значение:", QLineEdit::Normal,
                                            item->text(), &ok);
        if (ok && !txt.trimmed().isEmpty()) {
            item->setText(txt.trimmed());
        }
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
        ui->listWidget_delayTimeRegulatory->setCurrentRow(ui->listWidget_delayTimeRegulatory->count() - 1);
    }
}

void CyclicTestSettings::onEditDelayClicked()
{
    if (auto *item = ui->listWidget_delayTimeRegulatory->currentItem()) {
        bool ok;
        int v = QInputDialog::getInt(this, "Изменить задержку",
                                     "Новое значение (сек):",
                                     item->text().toInt(), 1, 3600, 1, &ok);
        if (ok) {
            item->setText(QString::number(v));
        }
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
        ui->listWidget_delayTimeShutOff->setCurrentRow(ui->listWidget_delayTimeShutOff->count() - 1);
    }
}

void CyclicTestSettings::onEditDelayShutOffClicked()
{
    if (auto *item = ui->listWidget_delayTimeShutOff->currentItem()) {
        bool ok;
        int v = QInputDialog::getInt(this, "Изменить задержку (Отсечной)",
                                     "Новое значение (сек):",
                                     item->text().toInt(), 1, 3600, 1, &ok);
        if (ok) {
            item->setText(QString::number(v));
        }
    }
}

void CyclicTestSettings::onRemoveDelayShutOffClicked()
{
    delete ui->listWidget_delayTimeShutOff->currentItem();
}

// --- Общая обработка «Старт»

void CyclicTestSettings::onPushButtonStartClicked()
{
    using TP = TestParameters;

    // --- Regulatory или Combined
    if (m_parameters.testType == TP::Regulatory || m_parameters.testType == TP::Combined) {
        // последовательность
        if (!ui->listWidget_testRangeRegulatory->currentItem()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Выберите хотя бы одну последовательность (регулирующий).");
            return;
        }
        QString sReg = ui->listWidget_testRangeRegulatory->currentItem()->text().trimmed();
        auto m1 = RegexPatterns::digitsHyphens().match(sReg);
        if (!m1.hasMatch() || m1.capturedLength() != sReg.length()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Регулирующая последовательность: только цифры и дефисы.");
            return;
        }
        m_parameters.regulatory_sequence = sReg;

        // задержка
        if (!ui->listWidget_delayTimeRegulatory->currentItem()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Выберите время задержки (регулирующий).");
            return;
        }
        m_parameters.regulatory_delaySec =
            ui->listWidget_delayTimeRegulatory->currentItem()->text().toInt();

        // удержание
        {
            QTime hold = ui->timeEdit_retentionTimeRegulatory->time();
            int secs = hold.minute()*60 + hold.second();
            if (secs <= 0) {
                QMessageBox::warning(this, "Ошибка",
                                    "Время удержания (регулирующий) > 00:00.");
                return;
            }
            m_parameters.regulatory_holdTimeSec = secs;
        }

        // число циклов
        {
            QString txt = ui->lineEdit_numberCyclesRegulatory->text().trimmed();
            auto m3 = RegexPatterns::digits().match(txt);
            if (!m3.hasMatch() || txt.isEmpty() || txt.toInt() <= 0) {
                QMessageBox::warning(this, "Ошибка",
                                     "Число циклов (регулирующий): введите положительное целое.");
                return;
            }
            m_parameters.regulatory_numCycles = txt.toInt();
        }

    }
    else {
        // сбросить ненужные
        m_parameters.regulatory_sequence.clear();
        m_parameters.regulatory_delaySec = 0;
        m_parameters.regulatory_holdTimeSec = 0;
        m_parameters.regulatory_numCycles = 0;
    }

    // --- Shutoff или Combined
    if (m_parameters.testType == TP::Shutoff || m_parameters.testType == TP::Combined) {
        // последовательность
        if (!ui->listWidget_testRangeShutOff->currentItem()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Выберите хотя бы одну последовательность (отсечной).");
            return;
        }
        QString sOff = ui->listWidget_testRangeShutOff->currentItem()->text().trimmed();
        auto m4 = RegexPatterns::digitsHyphens().match(sOff);
        if (!m4.hasMatch() || m4.capturedLength() != sOff.length()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Отсечной: только цифры и дефисы.");
            return;
        }
        m_parameters.shutoff_sequence = sOff;

        // задержка
        if (!ui->listWidget_delayTimeShutOff->currentItem()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Выберите время задержки (отсечной).");
            return;
        }
        m_parameters.shutoff_delaySec =
            ui->listWidget_delayTimeShutOff->currentItem()->text().toInt();

        // удержание
        {
            QTime hold = ui->timeEdit_retentionTimeShutOff->time();
            int secs = hold.minute()*60 + hold.second();
            if (secs <= 0) {
                QMessageBox::warning(this, "Ошибка",
                                     "Время удержания (отсечной) > 00:00.");
                return;
            }
            m_parameters.shutoff_holdTimeSec = secs;
        }

        // число циклов
        {
            QString txt = ui->lineEdit_numberCyclesShutOff->text().trimmed();
            auto m6 = RegexPatterns::digits().match(txt);
            if (!m6.hasMatch() || txt.isEmpty() || txt.toInt() <= 0) {
                QMessageBox::warning(this, "Ошибка",
                                     "Число циклов (отсечной): введите положительное целое.");
                return;
            }
            m_parameters.shutoff_numCycles = txt.toInt();
        }

        // 20mA
        m_parameters.shutoff_enable_20mA = ui->checkBox_20mA_enable->isChecked();

        m_parameters.shutoff_DO[0] = ui->pushButton_DO0_ShutOff->isChecked();
        m_parameters.shutoff_DO[1] = ui->pushButton_DO1_ShutOff->isChecked();
        m_parameters.shutoff_DO[2] = ui->pushButton_DO2_ShutOff->isChecked();
        m_parameters.shutoff_DO[3] = ui->pushButton_DO3_ShutOff->isChecked();

        m_parameters.shutoff_DI[0] = ui->checkBox_switch_3_0_ShutOff->isChecked();
        m_parameters.shutoff_DI[1] = ui->checkBox_switch_0_3_ShutOff->isChecked();
    }
    else {
        // сбросить ненужные
        m_parameters.shutoff_sequence.clear();
        m_parameters.shutoff_delaySec    = 0;
        m_parameters.shutoff_holdTimeSec = 0;
        m_parameters.shutoff_numCycles   = 0;
        m_parameters.shutoff_enable_20mA = false;
    }

    accept();
}
