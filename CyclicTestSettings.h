#ifndef CYCLICTESTSETTINGS_H
#define CYCLICTESTSETTINGS_H

#pragma once
#include <QDialog>
#include <QTime>
#include <QVector>



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
        int delay_sec;
        int num_cycles;
    };

    TestParameters getParameters() const;

private slots:
    void onPushButtonStartClicked();
    void onPushButtonCancelClicked();

    void onAddValueClicked();
    void onEditValueClicked();
    void onRemoveValueClicked();

    void onAddDelayClicked();
    void onEditDelayClicked();
    void onRemoveDelayClicked();

private:
    Ui::CyclicTestSettings *ui;
    TestParameters m_parameters;
};

#endif // CYCLICTESTSETTINGS_H
