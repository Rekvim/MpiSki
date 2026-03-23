#pragma once

#include <memory>

class AbstractTestRunner;
class Mpi;
class Registry;
class QObject;

enum class RunnerKind {
    Main,
    Step,
    Stroke,
    Response,
    Resolution,
    CyclicReg,
    CyclicShutoff
};

class RunnerFactory
{
public:
    static std::unique_ptr<AbstractTestRunner>
    create(RunnerKind kind,
           Mpi& mpi,
           Registry& registry,
           QObject* parent);
};