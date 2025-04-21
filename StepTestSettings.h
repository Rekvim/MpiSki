#ifndef STEPTESTSETTINGS_H
#define STEPTESTSETTINGS_H

#include <QDialog>
#include "qdatetime.h"

namespace Ui {
class StepTestSettings;
}

class StepTestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit StepTestSettings(QWidget *parent = nullptr);
    ~StepTestSettings();
    void reverse();

    struct TestParameters
    {
        quint32 delay;
        quint32 test_value;
        QVector<qreal> points;
    };

    TestParameters getParameters();

private:
    Ui::StepTestSettings *ui;
    const QTime max_time = QTime(0, 4, 0, 0);
    const QTime min_time = QTime(0, 0, 5, 0);
    const int min_T_value = 1;
    const int max_T_value = 99;
};

#endif // STEPTESTSETTINGS_H
