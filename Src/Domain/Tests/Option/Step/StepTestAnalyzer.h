#pragma once

#include <QtGlobal>
#include <QVector>

#include "Src/Domain/Tests/IAnalyzer.h"

#include "Src/Domain/Measurement/Sample.h"
#include "StepTestResult.h"

class StepTestAnalyzer : public IAnalyzer
{
public:

    struct Config
    {
        qreal T_value = 86;
    };

public:

    void setConfig(const Config& cfg)
    {
        m_cfg = cfg;
    }

    void start() override;
    void onSample(const Sample& s) override;
    void finish() override;

    const QVector<StepTestResult>& result() const;

private:

    struct StepState
    {
        double from = 0; // — старое задание
        double to = 0; // — новое задание

        double threshold = 0; // — порог достижения T%

        bool up = true;

        quint64 startTime = 0; // — момент начала шага

        bool tReached = false; // — движение все еще есть?
        quint32 tTime = 0; // — время достижения

        double overshoot = -1e9; // — перерегулирование
    };

    Config m_cfg;

    QVector<StepTestResult> m_results;

    StepState m_state;

    double m_prevTask = qQNaN();
    bool m_hasStep = false;

    void startStep(double from, double to, quint64 time);
    void updateStep(double pos, quint64 time);
    void finishStep();
};