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
        quint32 holdTimeMs;
        QVector<quint16> values;
        QVector<quint32> delaysMs;
        quint32 numCycles;
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
