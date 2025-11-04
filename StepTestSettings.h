#ifndef STEPTESTSETTINGS_H
#define STEPTESTSETTINGS_H

#pragma once
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
    inline static const QTime m_maxTime = QTime(0, 4, 0, 0);
    inline static const QTime m_minTime = QTime(0, 0, 5, 0);
    inline static const int m_minTValue = 1;
    inline static const int m_maxTValue = 99;
};

#endif // STEPTESTSETTINGS_H
