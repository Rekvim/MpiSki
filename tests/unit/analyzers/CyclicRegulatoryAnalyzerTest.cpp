#include "CyclicRegulatoryAnalyzerTest.h"

#include <QtTest>
#include <QDebug>

#include "Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.h"
#include "SampleGenerator.h"

namespace
{
// Создание одной точки измерения.
Sample makeSample(double task, double position)
{
    Sample s{};
    s.taskPercent = task;
    s.positionPercent = position;
    return s;
}

// Заголовок сценария.
void printCaseHeader(const QString& title)
{
    qDebug().noquote() << "\n---" << title << "---";
}

// Краткий вывод по всем диапазонам результата.
void printRanges(const CyclicRegulatoryResult& result)
{
    qDebug() << "Количество диапазонов:" << result.ranges.size();

    for (int i = 0; i < result.ranges.size(); ++i)
    {
        const auto& r = result.ranges[i];

        qDebug() << "Диапазон" << i
                 << "| уставка =" << r.rangePercent
                 << "| maxForward =" << r.maxForwardPosition
                 << "| minReverse =" << r.minReversePosition
                 << "| цикл max =" << r.maxForwardCycle
                 << "| цикл min =" << r.minReverseCycle;
    }
}

// Краткий вывод по входным данным.
template<typename Container>
void printSamplesInfo(const Container& samples)
{
    qDebug() << "Количество точек:" << samples.size();

    if (!samples.empty()) {
        qDebug() << "Первая точка:"
                 << "task =" << samples.front().taskPercent
                 << "pos =" << samples.front().positionPercent;

        qDebug() << "Последняя точка:"
                 << "task =" << samples.back().taskPercent
                 << "pos =" << samples.back().positionPercent;
    }
}
}

// Проверка работы на большом числе точек.
void CyclicRegulatoryAnalyzerTest::testWithManyPoints()
{
    printCaseHeader("Много точек");

    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    auto samples = SampleGenerator::generateForwardSequence(
        {0, 0, 0, 0, 0,
         50, 50, 50, 50, 50,
         100, 100, 100, 100, 100},
        0.05);

    printSamplesInfo(samples);

    for (const auto& s : samples)
        analyzer.onSample(s);

    analyzer.finish();

    const auto& result = analyzer.result();
    printRanges(result);

    QCOMPARE(result.ranges.size(), 3);

    // Дополнительная проверка: диапазоны реально существуют.
    QCOMPARE(result.ranges[0].rangePercent, 0.0);
    QCOMPARE(result.ranges[1].rangePercent, 50.0);
    QCOMPARE(result.ranges[2].rangePercent, 100.0);
}

// Проверка максимума на прямом ходе.
void CyclicRegulatoryAnalyzerTest::testForwardMax()
{
    printCaseHeader("Максимум на прямом ходе");

    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    analyzer.onSample(makeSample(0,   0.0));
    analyzer.onSample(makeSample(50, 49.0));
    analyzer.onSample(makeSample(100, 100.0));

    analyzer.onSample(makeSample(0,   0.0));   // новый цикл
    analyzer.onSample(makeSample(50, 50.5));   // максимум
    analyzer.onSample(makeSample(100, 99.0));

    analyzer.finish();

    const auto& r = analyzer.result();
    printRanges(r);

    QVERIFY(r.ranges.size() == 3);
    QCOMPARE(r.ranges[1].maxForwardPosition, 50.5);
}

// Проверка минимума на обратном ходе.
void CyclicRegulatoryAnalyzerTest::testReverseMin()
{
    printCaseHeader("Минимум на обратном ходе");

    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {100, 50, 0};

    analyzer.start();
    analyzer.configure(params);

    analyzer.onSample(makeSample(100, 100.0));
    analyzer.onSample(makeSample(50, 50.0));
    analyzer.onSample(makeSample(0,   0.0));

    analyzer.onSample(makeSample(100, 100.0));
    analyzer.onSample(makeSample(50,  48.8)); // минимум
    analyzer.onSample(makeSample(0,   1.0));

    analyzer.finish();

    const auto& r = analyzer.result();
    printRanges(r);

    QVERIFY(r.ranges.size() > 1);
    QCOMPARE(r.ranges[1].minReversePosition, 48.8);
}

// Проверка случая только прямого хода.
void CyclicRegulatoryAnalyzerTest::testOnlyForward()
{
    printCaseHeader("Только прямой ход");

    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    analyzer.onSample(makeSample(0,   0.0));
    analyzer.onSample(makeSample(50, 50.2));
    analyzer.onSample(makeSample(100, 99.9));

    analyzer.finish();

    const auto& r = analyzer.result();
    printRanges(r);

    QVERIFY(r.ranges.size() == 3);

    QVERIFY(qIsNaN(r.ranges[0].minReversePosition));
    QVERIFY(qIsNaN(r.ranges[1].minReversePosition));
    QVERIFY(qIsNaN(r.ranges[2].minReversePosition));
}

// Проверка устойчивости экстремума.
void CyclicRegulatoryAnalyzerTest::testExtremumStability()
{
    printCaseHeader("Устойчивость экстремума");

    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    // Несколько значений на одном шаге.
    analyzer.onSample(makeSample(0,  0.0));
    analyzer.onSample(makeSample(50, 49.0));
    analyzer.onSample(makeSample(50, 51.0));  // максимум
    analyzer.onSample(makeSample(50, 50.5));  // не должен заменить максимум

    analyzer.finish();

    const auto& r = analyzer.result();
    printRanges(r);

    QVERIFY(r.ranges.size() > 1);
    QCOMPARE(r.ranges[1].maxForwardPosition, 51.0);
}