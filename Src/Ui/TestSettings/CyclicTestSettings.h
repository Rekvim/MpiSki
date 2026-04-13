#pragma once

#include <QDialog>
#include <QTime>

#include "AbstractTestSettings.h"
#include "Src/Ui/Setup/SelectTests.h"
#include "Src/Domain/Tests/Cyclic/CyclicTestParams.h"

namespace Ui {
class CyclicTestSettings;
}

class CyclicTestSettings : public AbstractTestSettings
{
    Q_OBJECT

public:

    explicit CyclicTestSettings(QWidget* parent = nullptr);
    ~CyclicTestSettings() override;

    void applyPattern(SelectTests::PatternType pattern) override;
    void applyValveInfo(const ValveInfo& info) override;

    CyclicTestParams parameters() const { return m_params; }

private slots:
    void onTestSelectionChanged();
    void on_pushButton_start_clicked();

    void on_pushButton_addRangeRegulatory_clicked();
    void on_pushButton_editRangeRegulatory_clicked();
    void on_pushButton_removeRangeRegulatory_clicked();

    void on_pushButton_addDelayRegulatory_clicked();
    void on_pushButton_editDelayRegulatory_clicked();
    void on_pushButton_removeDelayRegulatory_clicked();

    void on_pushButton_addDelayShutOff_clicked();
    void on_pushButton_editDelayShutOff_clicked();
    void on_pushButton_removeDelayShutOff_clicked();

private:
    Ui::CyclicTestSettings* ui = nullptr;
    CyclicTestParams m_params;
    SelectTests::PatternType m_pattern = SelectTests::Pattern_None;

private:
    void initUi();
    void initConnections();
    void initDefaults();

    void setPattern(SelectTests::PatternType pattern);
    void updateVisibilityBySelectedTest();

    void fillDefaultRegulatoryPresets();
    void fillDefaultShutOffPresets();
    void bindRegulatoryPresetEditor();

    bool readParamsFromUi(CyclicTestParams& outParams);
    bool readRegulatoryParams(CyclicTestParams& outParams);
    bool readShutoffParams(CyclicTestParams& outParams);

    bool validatePositiveInt(const QString& text,
                             int& value,
                             const QString& fieldTitle) const;

    void showWarning(const QString& text) const;

    inline static const QTime kMaxHold = QTime(0, 1, 5, 0);
    inline static const QTime kMinHold = QTime(0, 0, 0, 0);

protected:
    QVector<qreal>& sequence() override
    {
        static QVector<qreal> dummy;
        return dummy;
    }

    QListWidget* sequenceListWidget() override
    {
        return nullptr;
    }
};
