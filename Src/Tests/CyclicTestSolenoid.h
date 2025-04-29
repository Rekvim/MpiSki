#pragma once
#include "Test.h"
#include <QTimer>
#include <QElapsedTimer>

class CyclicTestSolenoid : public MainTest
{
    Q_OBJECT

public:
    explicit CyclicTestSolenoid(QObject *parent = nullptr);

    void SetParameters(quint32 holdTimeMs,
                       const QVector<quint16> &values,
                       const QVector<quint32> &delaysMs,
                       quint32 numCycles);

public slots:
    void Process() override;

signals:
    void SetStartTime();
    void SolenoidResults(double, double, quint32, double, double);

private:
    quint32 m_holdTimeMs = 0;
    quint32 m_numCycles = 0;
    QTimer *m_cyclicGraphTimer = nullptr;
    QVector<quint16> m_values;
    QVector<quint32> m_delaysMs;
};
