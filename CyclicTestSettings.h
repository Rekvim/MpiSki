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

        quint32 regulatory_delayMsecs = 0;
        quint16 regulatory_holdTimeMsecs = 0;
        bool regulatory_enable_20mA = false;

        QVector<quint16> offSeqValues;
        QVector<quint16> rawOffValues;
        quint16 shutoff_numCycles = 0;

        quint32 shutoff_delayMsecs = 0;
        quint16 shutoff_holdTimeMsecs = 0;
        std::array<bool,4> shutoff_DO {{false,false,false,false}};
        bool shutoff_DI[2] {false,false};
    };

    TestParameters getParameters() const { return m_parameters; }

private slots:

    void onTestSelectionChanged();

    void onPushButtonStartClicked();

    void onAddValueClicked();
    void onEditValueClicked();
    void onRemoveValueClicked();
    void onAddDelayClicked();
    void onEditDelayClicked();
    void onRemoveDelayClicked();

    void onAddDelayShutOffClicked();
    void onEditDelayShutOffClicked();
    void onRemoveDelayShutOffClicked();

private:
    Ui::CyclicTestSettings *ui;
    TestParameters m_parameters;

    const QTime m_maxDeley = QTime(0, 4, 0, 0);
    const QTime m_minDeley = QTime(0, 0, 5, 0);

    const QTime m_maxHold = QTime(0, 1, 5, 0);
    const QTime m_minHold = QTime(0, 0, 0, 0);

    SelectTests::PatternType m_pattern = SelectTests::Pattern_None;
};

#endif // CYCLICTESTSETTINGS_H
