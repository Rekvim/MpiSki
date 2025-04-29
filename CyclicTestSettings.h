#ifndef CYCLICTESTSETTINGS_H
#define CYCLICTESTSETTINGS_H

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
        quint32          holdTimeMs;  // время удержания первого значения (мс)
        QVector<quint16> values;      // DAC-значения
        QVector<quint32> delaysMs;    // задержки между ними (мс)
        quint32          numCycles;   // сколько циклов
    };

    TestParameters getParameters() const { return m_parameters; }

private slots:
    void onPushButtonStartClicked();
    void onPushButtonCancelClicked();

private:
    Ui::CyclicTestSettings *ui;
    TestParameters m_parameters;
};

#endif // CYCLICTESTSETTINGS_H
