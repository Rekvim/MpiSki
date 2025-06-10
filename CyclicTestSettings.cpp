// CyclicTestSettings.cpp
#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"

#include "./Src/ValidatorFactory/RegexPatterns.h"

CyclicTestSettings::CyclicTestSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

    connect(ui->pushButton_add_value, &QPushButton::clicked, this, &CyclicTestSettings::onAddValueClicked);
    connect(ui->pushButton_edit_value, &QPushButton::clicked, this, &CyclicTestSettings::onEditValueClicked);
    connect(ui->pushButton_remove_value,&QPushButton::clicked, this, &CyclicTestSettings::onRemoveValueClicked);

    connect(ui->pushButton_add_delay, &QPushButton::clicked, this, &CyclicTestSettings::onAddDelayClicked);
    connect(ui->pushButton_edit_delay, &QPushButton::clicked, this, &CyclicTestSettings::onEditDelayClicked);
    connect(ui->pushButton_remove_delay, &QPushButton::clicked, this, &CyclicTestSettings::onRemoveDelayClicked);

    connect(ui->pushButton_cancel, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(ui->pushButton_start, &QPushButton::clicked,
            this, &CyclicTestSettings::onPushButtonStartClicked);
}

CyclicTestSettings::~CyclicTestSettings()
{
    delete ui;
}

void CyclicTestSettings::onAddValueClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Добавить последовательность",
                                         "Введите значения (через «-»):",
                                         QLineEdit::Normal, "", &ok);

    if (ok && !text.trimmed().isEmpty()) {
        ui->listWidget_value->addItem(text.trimmed());
        ui->listWidget_value->setCurrentRow(ui->listWidget_value->count()-1);
    }
}

void CyclicTestSettings::onEditValueClicked()
{
    auto *item = ui->listWidget_value->currentItem();
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
    delete ui->listWidget_value->currentItem();
}

void CyclicTestSettings::onAddDelayClicked()
{
    bool ok;
    int v = QInputDialog::getInt(this, "Добавить задержку",
                                 "Введите значение в секундах:", 10, 1, 3600, 1, &ok);

    if (ok) {
        ui->listWidget_step->addItem(QString::number(v));
        ui->listWidget_step->setCurrentRow(ui->listWidget_step->count()-1);
    }
}

void CyclicTestSettings::onEditDelayClicked()
{
    auto *item = ui->listWidget_step->currentItem();
    if (!item) return;

    bool ok;
    int v = QInputDialog::getInt(this, "Изменить задержку",
                                 "Новое значение (сек):",
                                 item->text().toInt(), 1, 3600, 1, &ok);
    if (ok) {
        item->setText(QString::number(v));
    }

    // Сделайть в минутах
}

void CyclicTestSettings::onRemoveDelayClicked()
{
    delete ui->listWidget_step->currentItem();
}

void CyclicTestSettings::onPushButtonStartClicked()
{
    auto *sequenceItem = ui->listWidget_value->currentItem();
    if (!sequenceItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите или добавьте хотя бы одну последовательность.");
        return;
    }
    QString sequenceValue = sequenceItem->text().trimmed();
    {
        auto m = RegexPatterns::digitsHyphens().match(sequenceValue);
        if (!m.hasMatch() || m.capturedLength() != sequenceValue.length()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Последовательность должна состоять только из цифр и дефисов, без пробелов и других символов.");
            return;
        }
    }
    m_parameters.sequence = sequenceValue;

    auto *delayItem = ui->listWidget_step->currentItem();
    if (!delayItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите время задержки.");
        return;
    }
    QString delayStr = delayItem->text().trimmed();
    {
        auto m = RegexPatterns::digits().match(delayStr);
        if (!m.hasMatch() || m.capturedLength() != delayStr.length()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Время задержки должно быть положительным числом в секундах.");
            return;
        }
    }
    m_parameters.delay_sec = delayStr.toInt();

    QTime retentionTime = ui->timeEdit->time();
    int holdSec = retentionTime.minute() * 60 + retentionTime.second();
    if (holdSec <= 0) {
        QMessageBox::warning(this, "Ошибка", "Время удержания должно быть больше 00:00.");
        return;
    }
    m_parameters.hold_time_sec = holdSec;

    QString cyclesStr = ui->lineEdit_number_cycles->text().trimmed();
    {
        auto m = RegexPatterns::digits().match(cyclesStr);
        if (!m.hasMatch() || m.capturedLength() != cyclesStr.length() || cyclesStr.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите корректное положительное целое число циклов.");
            return;
        }
    }
    int cycles = cyclesStr.toInt();
    if (cycles <= 0) {
        QMessageBox::warning(this, "Ошибка", "Число циклов должно быть больше нуля.");
        return;
    }
    m_parameters.num_cycles = cycles;

    accept();
}
