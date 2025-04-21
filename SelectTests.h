#ifndef SELECTTESTS_H
#define SELECTTESTS_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class SelectTests;
}

class SelectTests : public QDialog
{
    Q_OBJECT

public:
    explicit SelectTests(QWidget *parent = nullptr);
    ~SelectTests();
    struct BlockCTS
    {
        bool pressure_1;
        bool pressure_2;
        bool pressure_3;
        bool moving;
        bool input_4_20_mA;
        bool output_4_20_mA;
        bool usb;
        bool imit_switch_0_3;
        bool imit_switch_3_0;
        bool do_1;
        bool do_2;
        bool do_3;
        bool do_4;
    };

    BlockCTS getCTS() const;

private slots:
    void onCheckBoxChanged();
    bool isValidPattern(const BlockCTS& block_cts);

    void resetCheckBoxes();

    void ButtonClick_CTV();
    void ButtonClick_BTSV();
    void ButtonClick_CTSV();
    void ButtonClick_BTCV();
    void ButtonClick_CTCV();

    void ButtonClick();

private:
    Ui::SelectTests *ui;
    BlockCTS block_cts_;
};

#endif // SELECTTESTS_H
