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

    struct TestParameters {
        QString sequence;
        int     delay_sec;
        int     hold_time_sec;
        int     num_cycles;

        QString shutoff_sequence;
        int     shutoff_delay_sec;
        int     shutoff_hold_time_sec;
        int     shutoff_num_cycles;
        bool    shutoff_enable_20mA;
        bool    shutoff_DO[4];
        bool    shutoff_DI[2];
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
