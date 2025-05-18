#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"

#include "./src/Tests/CyclicTestSolenoid.h"

#include <QMessageBox>
#include <QInputDialog>

CyclicTestSettings::CyclicTestSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

    connect(ui->pushButton_add_value, &QPushButton::clicked, this, &CyclicTestSettings::onAddValueClicked);
    connect(ui->pushButton_edit_value, &QPushButton::clicked, this, &CyclicTestSettings::onEditValueClicked);
    connect(ui->pushButton_remove_value, &QPushButton::clicked, this, &CyclicTestSettings::onRemoveValueClicked);

    connect(ui->pushButton_add_delay, &QPushButton::clicked, this, &CyclicTestSettings::onAddDelayClicked);
    connect(ui->pushButton_edit_delay, &QPushButton::clicked, this, &CyclicTestSettings::onEditDelayClicked);
    connect(ui->pushButton_remove_delay, &QPushButton::clicked, this, &CyclicTestSettings::onRemoveDelayClicked);
}

CyclicTestSettings::~CyclicTestSettings()
{
    delete ui;
}

CyclicTestSettings::TestParameters CyclicTestSettings::getParameters() const {
    TestParameters p;
    if (ui->listWidget_value->currentItem())
        p.sequence = ui->listWidget_value->currentItem()->text();
    if (ui->listWidget_step->currentItem())
        p.delay_sec = ui->listWidget_step->currentItem()->text().toInt();
    p.num_cycles = ui->lineEdit_number_cycles->text().toInt();
    return p;
}

void CyclicTestSettings::onPushButtonStartClicked()
{
    QListWidgetItem *valueItem = ui->listWidget_value->currentItem();
    if (!valueItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите значения для испытания.");
        return;
    }
    QString sequence = valueItem->text();

    QListWidgetItem *delayItem = ui->listWidget_step->currentItem();
    if (!delayItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите время задержки.");
        return;
    }
    int delaySec = delayItem->text().toInt();

    bool ok = false;
    int numCycles = ui->lineEdit_number_cycles->text().toInt(&ok);
    if (!ok || numCycles <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите корректное количество циклов.");
        return;
    }

    auto *test = new CyclicTestSolenoid(this);
    test->SetParameters(sequence, delaySec, numCycles);

    accept();
}

void CyclicTestSettings::onAddValueClicked() {
    bool ok;
    QString text = QInputDialog::getText(this, "Добавить последовательность", "Введите значения:", QLineEdit::Normal, "", &ok);
    if (ok && !text.trimmed().isEmpty()) {
        ui->listWidget_value->addItem(text.trimmed());
    }
}

void CyclicTestSettings::onEditValueClicked() {
    auto *item = ui->listWidget_value->currentItem();
    if (!item) return;

    bool ok;
    QString text = QInputDialog::getText(this, "Изменить последовательность", "Измените значения:", QLineEdit::Normal, item->text(), &ok);
    if (ok && !text.trimmed().isEmpty()) {
        item->setText(text.trimmed());
    }
}

void CyclicTestSettings::onRemoveValueClicked() {
    auto *item = ui->listWidget_value->currentItem();
    if (item) {
        delete item;
    }
}


void CyclicTestSettings::onAddDelayClicked() {
    bool ok;
    int value = QInputDialog::getInt(this, "Добавить задержку", "Введите значение (сек):", 10, 1, 3600, 1, &ok);
    if (ok) {
        ui->listWidget_step->addItem(QString::number(value));
    }
}

void CyclicTestSettings::onEditDelayClicked() {
    auto *item = ui->listWidget_step->currentItem();
    if (!item) return;

    bool ok;
    int value = QInputDialog::getInt(this, "Изменить задержку", "Введите новое значение (сек):", item->text().toInt(), 1, 3600, 1, &ok);
    if (ok) {
        item->setText(QString::number(value));
    }
}

void CyclicTestSettings::onRemoveDelayClicked() {
    auto *item = ui->listWidget_step->currentItem();
    if (item) {
        delete item;
    }
}

void CyclicTestSettings::onPushButtonCancelClicked()
{
    reject();
}
