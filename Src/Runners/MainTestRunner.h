#ifndef MAINTESTRUNNER_H
#define MAINTESTRUNNER_H

#pragma once
#include <QObject>
#include "BaseRunner.h"
#include "./MainTestSettings.h"

class MainTestRunner : public BaseRunner {
    Q_OBJECT
public:
    using BaseRunner::BaseRunner;

signals:
    void getParameters_mainTest(struct MainTestSettings::TestParameters&);

protected:
    RunnerConfig buildConfig() override;
    void wireSpecificSignals(Test& t) override;
};

#endif // MAINTESTRUNNER_H
