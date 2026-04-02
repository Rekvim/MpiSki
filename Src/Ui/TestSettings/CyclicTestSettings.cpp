#include "CyclicTestSettings.h"
#include "ui_CyclicTestSettings.h"

#include "Src/Storage/Registry.h"
#include "Src/ValidatorFactory/RegexPatterns.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QPushButton>
#include <QListWidgetItem>

namespace CyclicSequenceUtils
{

bool parseSequence(const QString& src,
                   QVector<qreal>& dst,
                   QString& error)
{
    const QString s = src.trimmed();

    if (s.isEmpty()) {
        error = QStringLiteral("Последовательность пустая.");
        return false;
    }

    if (!RegexPatterns::floatSequence().match(s).hasMatch()) {
        error = QStringLiteral(
            "Формат: X или X-Y-Z, где значения — числа от 0 до 100."
            );
        return false;
    }

    dst.clear();

    const QStringList parts = s.split(QRegularExpression("\\s*-\\s*"),
                                      Qt::SkipEmptyParts);

    for (const QString& part : parts) {
        bool ok = false;
        const qreal value = part.toDouble(&ok);

        if (!ok) {
            error = QStringLiteral("Не удалось преобразовать число: \"%1\".").arg(part);
            dst.clear();
            return false;
        }

        if (value < 0.0 || value > 100.0) {
            error = QStringLiteral("Значение \"%1\" вне диапазона 0–100.").arg(part);
            dst.clear();
            return false;
        }

        dst.push_back(value);
    }

    return true;
}

QString reverseSequenceString(const QString& src)
{
    QStringList parts = src.split('-', Qt::SkipEmptyParts);

    for (QString& part : parts) {
        bool ok = false;
        const double value = part.trimmed().toDouble(&ok);

        if (!ok)
            return src;

        const double reversed = 100.0 - value;

        if (qFuzzyCompare(reversed + 1.0, std::round(reversed) + 1.0))
            part = QString::number(static_cast<int>(std::round(reversed)));
        else
            part = QString::number(reversed, 'g', 12);
    }

    return parts.join('-');
}

} // namespace CyclicSequenceUtils

CyclicTestSettings::CyclicTestSettings(QWidget* parent)
    : AbstractTestSettings(parent)
    , ui(new Ui::CyclicTestSettings)
{
    ui->setupUi(this);

    initUi();
    initConnections();
    initDefaults();

    onTestSelectionChanged();
}

CyclicTestSettings::~CyclicTestSettings()
{
    delete ui;
}

void CyclicTestSettings::initUi()
{
    clampTime(ui->timeEdit_retentionTimeRegulatory, kMinHold, kMaxHold);
    clampTime(ui->timeEdit_retentionTimeShutOff, kMinHold, kMaxHold);
}

void CyclicTestSettings::initConnections()
{
    connect(ui->pushButton_cancel,
            &QPushButton::clicked,
            this,
            &QDialog::reject);

    connect(ui->comboBox_testSelection,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &CyclicTestSettings::onTestSelectionChanged);
}

void CyclicTestSettings::initDefaults()
{
    fillDefaultRegulatoryPresets();
    fillDefaultShutOffPresets();
    bindRegulatoryPresetEditor();
}

void CyclicTestSettings::applyPattern(SelectTests::PatternType pattern)
{
    setPattern(pattern);
}

void CyclicTestSettings::applyValveInfo(const ValveInfo& info)
{
    if (info.safePosition == SafePosition::NormallyClosed)
        return;

    for (int i = 0; i < ui->listWidget_testRangeRegulatory->count(); ++i) {
        QListWidgetItem* item = ui->listWidget_testRangeRegulatory->item(i);
        if (!item)
            continue;

        item->setText(CyclicSequenceUtils::reverseSequenceString(item->text()));
    }
}

void CyclicTestSettings::setPattern(SelectTests::PatternType pattern)
{
    ui->comboBox_testSelection->clear();
    m_pattern = pattern;

    auto add = [&](const QString& label, CyclicTestParams::Type type)
    {
        ui->comboBox_testSelection->addItem(label, static_cast<int>(type));
    };

    switch (m_pattern) {
    case SelectTests::Pattern_B_CVT:
    case SelectTests::Pattern_C_CVT:
        add(QStringLiteral("Регулирующий"), CyclicTestParams::Regulatory);
        break;

    case SelectTests::Pattern_C_SOVT:
        add(QStringLiteral("Отсечной"), CyclicTestParams::Shutoff);
        break;

    case SelectTests::Pattern_B_SACVT:
    case SelectTests::Pattern_C_SACVT:
        add(QStringLiteral("Запорно-регулирующий"), CyclicTestParams::Combined);
        add(QStringLiteral("Регулирующий"), CyclicTestParams::Regulatory);
        add(QStringLiteral("Отсечной"), CyclicTestParams::Shutoff);
        break;

    default:
        break;
    }

    ui->comboBox_testSelection->setCurrentIndex(0);
    onTestSelectionChanged();
}

void CyclicTestSettings::onTestSelectionChanged()
{
    updateVisibilityBySelectedTest();
}

void CyclicTestSettings::updateVisibilityBySelectedTest()
{
    const int idx = ui->comboBox_testSelection->currentIndex();
    if (idx < 0)
        return;

    const auto type = static_cast<CyclicTestParams::Type>(
        ui->comboBox_testSelection->itemData(idx).toInt());

    m_params.testType = type;

    const bool showReg = (type == CyclicTestParams::Regulatory ||
                          type == CyclicTestParams::Combined);

    const bool showOff = (type == CyclicTestParams::Shutoff ||
                          type == CyclicTestParams::Combined);

    ui->widget_retentionTimeRegulatory->setVisible(showReg);
    ui->widget_shutOff->setVisible(showOff);
}

void CyclicTestSettings::fillDefaultRegulatoryPresets()
{
    static const QStringList presetsRange = {
        QStringLiteral("0-25-50-75-100-75-50-25-0"),
        QStringLiteral("0-50-100-50-0"),
        QStringLiteral("0-100-0")
    };

    static const QStringList presetsDelay = {
        QStringLiteral("10"),
        QStringLiteral("30"),
        QStringLiteral("60")
    };

    ui->listWidget_testRangeRegulatory->clear();
    ui->listWidget_testRangeRegulatory->addItems(presetsRange);
    ui->listWidget_testRangeRegulatory->setCurrentRow(0);

    ui->listWidget_delayTimeRegulatory->clear();
    ui->listWidget_delayTimeRegulatory->addItems(presetsDelay);
    ui->listWidget_delayTimeRegulatory->setCurrentRow(0);

    ui->lineEdit_numberCyclesRegulatory->setText(QStringLiteral("10"));
}

void CyclicTestSettings::fillDefaultShutOffPresets()
{
    static const QStringList presetsRange = {
        QStringLiteral("0-100-0")
};

static const QStringList presetsDelay = {
    QStringLiteral("10"),
    QStringLiteral("30"),
    QStringLiteral("60")
};

ui->listWidget_testRangeShutOff->clear();
ui->listWidget_testRangeShutOff->addItems(presetsRange);
ui->listWidget_testRangeShutOff->setCurrentRow(0);

ui->listWidget_delayTimeShutOff->clear();
ui->listWidget_delayTimeShutOff->addItems(presetsDelay);
ui->listWidget_delayTimeShutOff->setCurrentRow(0);

ui->lineEdit_numberCyclesShutOff->setText(QStringLiteral("10"));
}

void CyclicTestSettings::bindRegulatoryPresetEditor()
{
    ui->pushButton_editRangeRegulatory->setEnabled(false);
    ui->pushButton_removeRangeRegulatory->setEnabled(false);

    connect(ui->listWidget_testRangeRegulatory,
            &QListWidget::currentRowChanged,
            this,
            [this](int row)
            {
                ui->pushButton_editRangeRegulatory->setEnabled(row >= 0);
                ui->pushButton_removeRangeRegulatory->setEnabled(
                    row >= 0 &&
                    ui->listWidget_testRangeRegulatory->count() > 1);
            });
}

void CyclicTestSettings::showWarning(const QString& text) const
{
    QMessageBox::warning(const_cast<CyclicTestSettings*>(this),
                         QStringLiteral("Ошибка"),
                         text);
}

bool CyclicTestSettings::validatePositiveInt(const QString& text,
                                             int& value,
                                             const QString& fieldTitle) const
{
    const QString trimmed = text.trimmed();

    if (trimmed.isEmpty()) {
        showWarning(fieldTitle + QStringLiteral(": значение не должно быть пустым."));
        return false;
    }

    const auto match = RegexPatterns::digits().match(trimmed);
    if (!match.hasMatch()) {
        showWarning(fieldTitle + QStringLiteral(": введите положительное целое число."));
        return false;
    }

    value = trimmed.toInt();
    if (value <= 0) {
        showWarning(fieldTitle + QStringLiteral(": число должно быть больше нуля."));
        return false;
    }

    return true;
}

bool CyclicTestSettings::readRegulatoryParams(CyclicTestParams& outParams)
{
    if (!ui->listWidget_testRangeRegulatory->currentItem()) {
        showWarning(QStringLiteral(
            "Выберите хотя бы одну последовательность (регулирующий)."));
        return false;
    }

    {
        const QString seqText =
            ui->listWidget_testRangeRegulatory->currentItem()->text().trimmed();

        QString error;
        if (!CyclicSequenceUtils::parseSequence(seqText, outParams.regSeqValues, error)) {
            showWarning(QStringLiteral("Регулирующая последовательность: ") + error);
            return false;
        }
    }

    if (!ui->listWidget_delayTimeRegulatory->currentItem()) {
        showWarning(QStringLiteral(
            "Выберите время задержки (регулирующий)."));
        return false;
    }

    outParams.regulatory_delayMs =
        ui->listWidget_delayTimeRegulatory->currentItem()->text().toUInt() * 1000U;

    outParams.regulatory_holdMs =
        ui->timeEdit_retentionTimeRegulatory->time().msecsSinceStartOfDay();

    {
        int cycles = 0;
        if (!validatePositiveInt(ui->lineEdit_numberCyclesRegulatory->text(),
                                 cycles,
                                 QStringLiteral("Число циклов (регулирующий)")))
            return false;

        outParams.regulatory_numCycles = static_cast<quint16>(cycles);
    }

    outParams.regulatory_enable_20mA =
        ui->checkBox_20mA_enable->isChecked();

    return true;
}

bool CyclicTestSettings::readShutoffParams(CyclicTestParams& outParams)
{
    if (!ui->listWidget_testRangeShutOff->currentItem()) {
        showWarning(QStringLiteral(
            "Выберите хотя бы одну последовательность (отсечной)."));
        return false;
    }

    // Если по бизнес-логике shutoff всегда фиксированный:
    outParams.offSeqValues = {0.0, 100.0, 0.0};

    if (!ui->listWidget_delayTimeShutOff->currentItem()) {
        showWarning(QStringLiteral(
            "Выберите время задержки (отсечной)."));
        return false;
    }

    outParams.shutoff_delayMs =
        ui->listWidget_delayTimeShutOff->currentItem()->text().toUInt() * 1000U;

    outParams.shutoff_holdMs =
        ui->timeEdit_retentionTimeShutOff->time().msecsSinceStartOfDay();

    {
        int cycles = 0;
        if (!validatePositiveInt(ui->lineEdit_numberCyclesShutOff->text(),
                                 cycles,
                                 QStringLiteral("Число циклов (отсечной)")))
            return false;

        outParams.shutoff_numCycles = static_cast<quint16>(cycles);
    }

    outParams.shutoff_DO[0] = ui->pushButton_DO0_ShutOff->isChecked();
    outParams.shutoff_DO[1] = ui->pushButton_DO1_ShutOff->isChecked();
    outParams.shutoff_DO[2] = ui->pushButton_DO2_ShutOff->isChecked();
    outParams.shutoff_DO[3] = ui->pushButton_DO3_ShutOff->isChecked();

    outParams.shutoff_DI[0] = ui->checkBox_switch_3_0_ShutOff->isChecked();
    outParams.shutoff_DI[1] = ui->checkBox_switch_0_3_ShutOff->isChecked();

    return true;
}

bool CyclicTestSettings::readParamsFromUi(CyclicTestParams& outParams)
{
    const int idx = ui->comboBox_testSelection->currentIndex();
    if (idx < 0) {
        showWarning(QStringLiteral("Не выбран тип испытания."));
        return false;
    }

    outParams = CyclicTestParams{};
    outParams.testType = static_cast<CyclicTestParams::Type>(
        ui->comboBox_testSelection->itemData(idx).toInt());

    const bool needReg = (outParams.testType == CyclicTestParams::Regulatory ||
                          outParams.testType == CyclicTestParams::Combined);

    const bool needOff = (outParams.testType == CyclicTestParams::Shutoff ||
                          outParams.testType == CyclicTestParams::Combined);

    if (needReg) {
        if (!readRegulatoryParams(outParams))
            return false;
    } else {
        outParams.clearRegulatory();
    }

    if (needOff) {
        if (!readShutoffParams(outParams))
            return false;
    } else {
        outParams.clearShutoff();
    }

    return true;
}

void CyclicTestSettings::on_pushButton_start_clicked()
{
    CyclicTestParams params;
    if (!readParamsFromUi(params))
        return;

    m_params = params;
    accept();
}

// ---------------------------
// Regulatory presets
// ---------------------------

void CyclicTestSettings::on_pushButton_addRangeRegulatory_clicked()
{
    bool ok = false;

    QString text = QInputDialog::getText(
        this,
        QStringLiteral("Добавить последовательность"),
        QStringLiteral("Введите значения (через «-»):"),
        QLineEdit::Normal,
        QString(),
        &ok
        );

    if (!ok || text.trimmed().isEmpty())
        return;

    QString error;
    QVector<qreal> tmp;
    if (!CyclicSequenceUtils::parseSequence(text.trimmed(), tmp, error)) {
        showWarning(error);
        return;
    }

    ui->listWidget_testRangeRegulatory->addItem(text.trimmed());
    ui->listWidget_testRangeRegulatory->setCurrentRow(
        ui->listWidget_testRangeRegulatory->count() - 1);
}

void CyclicTestSettings::on_pushButton_editRangeRegulatory_clicked()
{
    QListWidgetItem* item = ui->listWidget_testRangeRegulatory->currentItem();
    if (!item)
        return;

    bool ok = false;

    QString text = QInputDialog::getText(
        this,
        QStringLiteral("Изменить последовательность"),
        QStringLiteral("Новое значение:"),
        QLineEdit::Normal,
        item->text(),
        &ok
        );

    if (!ok || text.trimmed().isEmpty())
        return;

    QString error;
    QVector<qreal> tmp;
    if (!CyclicSequenceUtils::parseSequence(text.trimmed(), tmp, error)) {
        showWarning(error);
        return;
    }

    item->setText(text.trimmed());
}

void CyclicTestSettings::on_pushButton_removeRangeRegulatory_clicked()
{
    delete ui->listWidget_testRangeRegulatory->currentItem();
}

void CyclicTestSettings::on_pushButton_addDelayRegulatory_clicked()
{
    bool ok = false;

    const int value = QInputDialog::getInt(
        this,
        QStringLiteral("Добавить задержку"),
        QStringLiteral("Введите значение в секундах:"),
        10,
        1,
        3600,
        1,
        &ok
        );

    if (!ok)
        return;

    ui->listWidget_delayTimeRegulatory->addItem(QString::number(value));
    ui->listWidget_delayTimeRegulatory->setCurrentRow(
        ui->listWidget_delayTimeRegulatory->count() - 1);
}

void CyclicTestSettings::on_pushButton_editDelayRegulatory_clicked()
{
    QListWidgetItem* item = ui->listWidget_delayTimeRegulatory->currentItem();
    if (!item)
        return;

    bool ok = false;

    const int value = QInputDialog::getInt(
        this,
        QStringLiteral("Изменить задержку"),
        QStringLiteral("Новое значение (сек):"),
        item->text().toInt(),
        1,
        3600,
        1,
        &ok
        );

    if (!ok)
        return;

    item->setText(QString::number(value));
}

void CyclicTestSettings::on_pushButton_removeDelayRegulatory_clicked()
{
    delete ui->listWidget_delayTimeRegulatory->currentItem();
}

// ---------------------------
// Shutoff presets
// ---------------------------

void CyclicTestSettings::on_pushButton_addDelayShutOff_clicked()
{
    bool ok = false;

    const int value = QInputDialog::getInt(
        this,
        QStringLiteral("Добавить задержку (Отсечной)"),
        QStringLiteral("Введите значение в секундах:"),
        10,
        1,
        3600,
        1,
        &ok
        );

    if (!ok)
        return;

    ui->listWidget_delayTimeShutOff->addItem(QString::number(value));
    ui->listWidget_delayTimeShutOff->setCurrentRow(
        ui->listWidget_delayTimeShutOff->count() - 1);
}

void CyclicTestSettings::on_pushButton_editDelayShutOff_clicked()
{
    QListWidgetItem* item = ui->listWidget_delayTimeShutOff->currentItem();
    if (!item)
        return;

    bool ok = false;

    const int value = QInputDialog::getInt(
        this,
        QStringLiteral("Изменить задержку (Отсечной)"),
        QStringLiteral("Новое значение (сек):"),
        item->text().toInt(),
        1,
        3600,
        1,
        &ok
        );

    if (!ok)
        return;

    item->setText(QString::number(value));
}

void CyclicTestSettings::on_pushButton_removeDelayShutOff_clicked()
{
    delete ui->listWidget_delayTimeShutOff->currentItem();
}
