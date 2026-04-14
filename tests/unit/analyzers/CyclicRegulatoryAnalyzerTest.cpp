#include "CyclicRegulatoryAnalyzerTest.h"
#include "Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.h"
#include "SampleGenerator.h"

// Вспомогательная функция для создания образца
static Sample makeSample(double task, double position) {
    Sample s;
    s.taskPercent = task;
    s.positionPercent = position;
    return s;
}

void CyclicRegulatoryAnalyzerTest::testWithManyPoints()
{
    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    // Генерируем много точек для каждого шага
    auto samples = SampleGenerator::generateForwardSequence(
        {0, 0, 0, 0, 0,
         50, 50, 50, 50, 50,
         100, 100, 100, 100, 100},
        0.05
        );

    for (const auto& s : samples) {
        analyzer.onSample(s);
    }

    analyzer.finish();

    const auto& result = analyzer.result();
    QCOMPARE(result.ranges.size(), 3);
}

void CyclicRegulatoryAnalyzerTest::testForwardMax()
{
    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    // ACT
    analyzer.onSample(makeSample(0, 0.0));
    analyzer.onSample(makeSample(50, 49.0));
    analyzer.onSample(makeSample(100, 100.0));

    analyzer.onSample(makeSample(0, 0.0));   // новый цикл
    analyzer.onSample(makeSample(50, 50.5)); // максимум
    analyzer.onSample(makeSample(100, 99.0));

    analyzer.finish();

    // ASSERT
    const auto& r = analyzer.result();

    QVERIFY(r.ranges.size() == 3);

    QCOMPARE(r.ranges[1].maxForwardPosition, 50.5);
}

void CyclicRegulatoryAnalyzerTest::testReverseMin()
{
    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {100, 50, 0};

    analyzer.start();
    analyzer.configure(params);

    // ACT
    analyzer.onSample(makeSample(100, 100.0));
    analyzer.onSample(makeSample(50, 50.0));
    analyzer.onSample(makeSample(0, 0.0));

    analyzer.onSample(makeSample(100, 100.0));
    analyzer.onSample(makeSample(50, 48.8)); // минимум
    analyzer.onSample(makeSample(0, 1.0));

    analyzer.finish();

    const auto& r = analyzer.result();

    QCOMPARE(r.ranges[1].minReversePosition, 48.8);
}

void CyclicRegulatoryAnalyzerTest::testOnlyForward()
{
    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    analyzer.onSample(makeSample(0, 0.0));
    analyzer.onSample(makeSample(50, 50.2));
    analyzer.onSample(makeSample(100, 99.9));

    analyzer.finish();

    const auto& r = analyzer.result();

    QVERIFY(qIsNaN(r.ranges[0].minReversePosition));
    QVERIFY(qIsNaN(r.ranges[1].minReversePosition));
    QVERIFY(qIsNaN(r.ranges[2].minReversePosition));
}

void CyclicRegulatoryAnalyzerTest::testExtremumStability()
{
    CyclicRegulatoryAnalyzer analyzer;

    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};

    analyzer.start();
    analyzer.configure(params);

    // ACT (несколько значений для одного шага)
    analyzer.onSample(makeSample(0, 0.0));
    analyzer.onSample(makeSample(50, 49.0));
    analyzer.onSample(makeSample(50, 51.0)); // max
    analyzer.onSample(makeSample(50, 50.5)); // не должен заменить max

    analyzer.finish();

    const auto& r = analyzer.result();

    QCOMPARE(r.ranges[1].maxForwardPosition, 51.0);
}