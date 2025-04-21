#ifndef CYCLICTESTSETTINGS_H
#define CYCLICTESTSETTINGS_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
class CyclicTestSettings;
}

class CyclicTestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit CyclicTestSettings(QWidget *parent = nullptr);
    ~CyclicTestSettings();
    void reverse();

    struct TestParameters {
        quint32 delay;
        quint32 test_value;
        quint16 num_cycles;
        QVector<qreal> points;
        QVector<qreal> steps;

        quint64 time_forward_last = 0;
        quint64 time_backward_last = 0;
        quint16 num_cycles_done = 0;
        qreal range_percent = 0;
        quint64 total_time = 0;
    };

    TestParameters getParameters();

private:
    Ui::CyclicTestSettings *ui;
    const QTime m_maxTime = QTime(0, 4, 0, 0);
    const QTime m_minTime = QTime(0, 0, 5, 0);
};

#endif // CYCLICTESTSETTINGS_H
