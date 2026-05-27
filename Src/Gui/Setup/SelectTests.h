#pragma once

#include <QDialog>
#include <QCheckBox>

#include "Domain/DeviceProfile.h"

namespace Ui {
class SelectTests;
}

class SelectTests : public QDialog
{
    Q_OBJECT

public:
    explicit SelectTests(QWidget *parent = nullptr);
    ~SelectTests();

    Domain::DeviceProfile profile() const;

private:
    enum PatternType {
        Pattern_None,
        Pattern_C_SOVT,
        Pattern_B_SACVT,
        Pattern_C_SACVT,
        Pattern_B_CVT,
        Pattern_C_CVT
    };

    struct PatternSetup {
        QList<QCheckBox*> checksOn;
        QList<QCheckBox*> checksOff;
    };

    struct BlockCTS
    {
        bool pressure_1 = false;
        bool pressure_2 = false;
        bool pressure_3 = false;
        bool moving = false;
        bool input_4_20_mA = false;
        bool output_4_20_mA = false;
        bool usb = false;
        bool imit_switch_0_3 = false;
        bool imit_switch_3_0 = false;
        bool do_1 = false;
        bool do_2 = false;
        bool do_3 = false;
        bool do_4 = false;
    };

private slots:
    void onCheckBoxChanged();

    void ButtonClick_C_SOVT();
    void ButtonClick_B_SACVT();
    void ButtonClick_C_SACVT();
    void ButtonClick_B_CVT();
    void ButtonClick_C_CVT();

    void ButtonClick();

private:
    Ui::SelectTests *ui;
    BlockCTS m_blockCts;
    PatternType m_currentPattern = Pattern_None;
    PatternType detectCurrentPattern() const;

    bool m_suppressDebugOutput = false;

    void setPattern(const PatternSetup& setup);
    QList<QCheckBox*> allCheckBoxes() const;
};
