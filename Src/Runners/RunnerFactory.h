#ifndef RUNNERFACTORY_H
#define RUNNERFACTORY_H

#pragma once
#include <QObject>
#include <memory>

class ITestRunner; class MPI; class Registry; class QObject;

enum class RunnerKind {
    Main, Step, Stroke, Response, Resolution, CyclicReg, CyclicShutoff, CyclicCombined
};

class RunnerFactory {
public:
    static std::unique_ptr<ITestRunner>
    create(RunnerKind kind, MPI& mpi, Registry& reg, QObject* parent);
};


#endif // RUNNERFACTORY_H
