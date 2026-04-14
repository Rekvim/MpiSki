#pragma once

#include "Src/Domain/Measurement/Sample.h"

class SampleGenerator {
public:
    static std::vector<Sample> generateForwardSequence(
        const std::vector<double>& tasks,
        double noiseLevel = 0.0,
        int pointsPerStep = 1)
    {
        std::vector<Sample> samples;

        for (double task : tasks) {
            for (int i = 0; i < pointsPerStep; ++i) {
                Sample s;
                s.taskPercent = task;

                // Идеальное положение = заданию
                double idealPos = task;

                // Добавляем небольшой шум если нужно
                if (noiseLevel > 0) {
                    double noise = ((double)rand() / RAND_MAX - 0.5) * 2 * noiseLevel;
                    idealPos += noise;
                }

                s.positionPercent = idealPos;
                samples.push_back(s);
            }
        }

        return samples;
    }

    static std::vector<Sample> generateCycle(
        const std::vector<double>& forwardTasks,  // {0, 50, 100}
        const std::vector<double>& reverseTasks,  // {100, 50, 0}
        int cycles = 2,
        double noiseLevel = 0.0)
    {
        std::vector<Sample> samples;

        for (int cycle = 0; cycle < cycles; ++cycle) {
            // Прямой ход
            for (double task : forwardTasks) {
                Sample s;
                s.taskPercent = task;
                s.positionPercent = task + (cycle == 1 ? 0.5 : 0.0); // имитация смещения
                if (noiseLevel > 0) {
                    s.positionPercent += ((double)rand() / RAND_MAX - 0.5) * 2 * noiseLevel;
                }
                samples.push_back(s);
            }

            // Обратный ход (если нужен)
            if (!reverseTasks.empty()) {
                for (double task : reverseTasks) {
                    Sample s;
                    s.taskPercent = task;
                    s.positionPercent = task;
                    if (noiseLevel > 0) {
                        s.positionPercent += ((double)rand() / RAND_MAX - 0.5) * 2 * noiseLevel;
                    }
                    samples.push_back(s);
                }
            }
        }

        return samples;
    }
};
