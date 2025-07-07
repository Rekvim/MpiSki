// CyclicTestSettings.h
#ifndef CYCLICTESTSETTINGS_H
#define CYCLICTESTSETTINGS_H

#pragma once
#include <QDialog>
#include <QTime>
#include <QMessageBox>
#include <QInputDialog>

namespace Ui {
class CyclicTestSettings;
}

class CyclicTestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit CyclicTestSettings(QWidget *parent = nullptr);
    ~CyclicTestSettings();

    enum AvailableTests {
        OnlyRegulatory,
        OnlyShutoff,
        ZipRegulatory
    };

    void setAvailableTests(AvailableTests at);

    struct TestParameters {
        enum Type { Regulatory, Shutoff, Combined } testType;
        QString regulatory_sequence;
        int regulatory_delaySec;
        int regulatory_holdTimeSec;
        int regulatory_numCycles;
        QString shutoff_sequence;
        int shutoff_delaySec;
        int shutoff_holdTimeSec;
        int shutoff_numCycles;
        bool shutoff_enable_20mA;
        std::array<bool,4> shutoff_DO;
        bool shutoff_DI[2];
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
};

#endif // CYCLICTESTSETTINGS_H
