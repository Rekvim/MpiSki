#ifndef OTHERTESTSETTINGS_H
#define OTHERTESTSETTINGS_H

#include <QInputDialog>
#include <QDialog>
#include "qdatetime.h"

namespace Ui {
class OtherTestSettings;
}

class OtherTestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit OtherTestSettings(QWidget *parent = nullptr);
    ~OtherTestSettings();
    void reverse();

    struct TestParameters
    {
        quint32 delay;
        QVector<qreal> points;
        QVector<qreal> steps;
    };

    TestParameters getParameters();

private:
    Ui::OtherTestSettings *ui;
    const QTime m_maxTime = QTime(0, 4, 0, 0);
    const QTime m_minTime = QTime(0, 0, 5, 0);
};

#endif // OTHERTESTSETTINGS_H
