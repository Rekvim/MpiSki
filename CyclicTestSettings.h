// CyclicTestSettings.h
#ifndef CYCLICTESTSETTINGS_H
#define CYCLICTESTSETTINGS_H

#pragma once
#include <QDialog>
#include <QTime>
#include <QMessageBox>
#include <QInputDialog>
#include "SelectTests.h"

namespace Ui {
class CyclicTestSettings;
}

class CyclicTestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit CyclicTestSettings(QWidget *parent = nullptr);
    ~CyclicTestSettings();

    void setPattern(SelectTests::PatternType pattern);

    struct TestParameters {
        enum Type { Regulatory, Shutoff, Combined } testType;

        QVector<quint16> regSeqValues;
        QVector<quint16> rawRegValues;
        quint16 regulatory_numCycles = 0;

        quint64 regulatory_delay = 0;
        quint64 regulatory_holdTime= 0;
        bool regulatory_enable_20mA = false;

        QVector<quint16> offSeqValues;
        QVector<quint16> rawOffValues;
        quint16 shutoff_numCycles = 0;

        quint64 shutoff_delay = 0;
        quint64 shutoff_holdTime= 0;
        std::array<bool,4> shutoff_DO {{false,false,false,false}};
        bool shutoff_DI[2] {false,false};
    };

    TestParameters getParameters() const { return m_parameters; }

private slots:

    void onTestSelectionChanged();

    void onPushButtonStartClicked();

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
    Ui::CyclicTestSettings *ui;
    TestParameters m_parameters;

    SelectTests::PatternType m_pattern = SelectTests::Pattern_None;
};

#endif // CYCLICTESTSETTINGS_H
