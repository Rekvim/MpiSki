#ifndef BASERUNNER_H
#define BASERUNNER_H

#include <QThread>
#include <QMetaObject>
#include <QObject>

#include "./Src/Tests/Test.h"
#include "./Src/Runners/ITestRunner.h"
#include "./Src/MPI/MPI.h"
#include "./Registry.h"


struct RunnerConfig {
    Test*    worker = nullptr;   // конкретный *Test (MainTest/StepTest/...)
    quint64  totalMs = 0;
    int      chartToClear = -1;  // если надо очистить
};

class BaseRunner : public ITestRunner {
    Q_OBJECT
public:
    BaseRunner(MPI& mpi, Registry& reg, QObject* parent=nullptr);
    ~BaseRunner() override;

public slots:
    void start() final;   // вызывает buildConfig(), заводит поток, вешает общие connect’ы
    void stop() override; // дергает worker->StoppingTheTest()

protected:
    virtual RunnerConfig buildConfig() = 0;          // собрать worker + задать totalMs/график
    virtual void wireSpecificSignals(Test& t) {}     // доцепить спец. сигналы текущего теста

    MPI& m_mpi; Registry& m_reg;

private:
    QThread* m_thread = nullptr;
    Test*    m_worker = nullptr;
};

#endif // BASERUNNER_H
