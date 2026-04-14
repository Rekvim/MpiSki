#include "CyclicRegulatoryAnalyzerTest.h"
#include "Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.h"
#include "Src/Domain/Measurement/Sample.h"

void CyclicRegulatoryAnalyzerTest::testConfigureWithFullSequence()
{
    qDebug() << "\n========== TEST 1: Configure with sequence ==========\n";
    
    CyclicRegulatoryAnalyzer analyzer;
    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 25, 50, 75, 100, 75, 50, 25, 0};
    
    analyzer.start();
    analyzer.configure(params);
    const auto& result = analyzer.result();
    
    qDebug() << "Sequence:" << params.regulatory.sequence;
    qDebug() << "Extracted ranges:" << result.ranges.size();
    
    QCOMPARE(result.ranges.size(), 5); // Только 0,25,50,75,100
    
    for (int i = 0; i < result.ranges.size(); ++i) {
        qDebug() << QString("  Range %1: %2%").arg(i).arg(result.ranges[i].rangePercent);
        QCOMPARE(result.ranges[i].rangePercent, i * 25.0);
    }
    
    qDebug() << "\n✓ Test passed!\n";
}

void CyclicRegulatoryAnalyzerTest::testIdealMotion()
{
    qDebug() << "\n========== TEST 2: Ideal motion (no hysteresis) ==========\n";
    
    CyclicRegulatoryAnalyzer analyzer;
    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 25, 50, 75, 100};
    params.regulatory.numCycles = 1;
    
    analyzer.start();
    analyzer.configure(params);
    
    // Идеальное движение: задача = позиция
    QVector<QPair<double, double>> testData = {
        {0, 0}, {25, 25}, {50, 50}, {75, 75}, {100, 100},  // forward
        {75, 75}, {50, 50}, {25, 25}, {0, 0}                // reverse
    };
    
    qDebug() << "Feeding" << testData.size() << "samples...";
    for (const auto& data : testData) {
        Sample s;
        s.taskPercent = data.first;
        s.positionPercent = data.second;
        analyzer.onSample(s);
        qDebug() << QString("  Task: %1% → Pos: %2%").arg(data.first, 5).arg(data.second, 5);
    }
    
    analyzer.finish();
    const auto& result = analyzer.result();
    
    qDebug() << "\nResults:";
    for (const auto& range : result.ranges) {
        double hysteresis = range.minReversePosition - range.maxForwardPosition;
        qDebug() << QString("  %1%: forward=%2, reverse=%3, hysteresis=%4")
                    .arg(range.rangePercent, 3)
                    .arg(range.maxForwardPosition, 6, 'f', 2)
                    .arg(range.minReversePosition, 6, 'f', 2)
                    .arg(hysteresis, 6, 'f', 2);
        
        // В идеале погрешность должна быть меньше 0.01%
        QVERIFY(qAbs(range.maxForwardPosition - range.rangePercent) < 0.01);
        QVERIFY(qAbs(range.minReversePosition - range.rangePercent) < 0.01);
    }
    
    qDebug() << "\n✓ Test passed!\n";
}

void CyclicRegulatoryAnalyzerTest::testHysteresisDetection()
{
    qDebug() << "\n========== TEST 3: Hysteresis detection (2% lag) ==========\n";
    
    CyclicRegulatoryAnalyzer analyzer;
    CyclicTestParams params;
    params.type = CyclicTestParams::Regulatory;
    params.regulatory.sequence = {0, 50, 100};
    params.regulatory.numCycles = 1;
    
    analyzer.start();
    analyzer.configure(params);
    
    // Данные с гистерезисом: forward отстает на 2%, reverse опережает на 2%
    QVector<QPair<double, double>> testData = {
        {0, 0},      // forward start
        {50, 48},    // отстает на 2%
        {100, 98},   // отстает на 2%
        {50, 52},    // опережает на 2%
        {0, 2}       // опережает на 2%
    };
    
    qDebug() << "Simulating 2% hysteresis (forward lags, reverse leads):";
    for (const auto& data : testData) {
        Sample s;
        s.taskPercent = data.first;
        s.positionPercent = data.second;
        analyzer.onSample(s);
        qDebug() << QString("  Task: %1% → Pos: %2%").arg(data.first, 5).arg(data.second, 5);
    }
    
    analyzer.finish();
    const auto& result = analyzer.result();
    
    qDebug() << "\nResults:";
    for (const auto& range : result.ranges) {
        double hysteresis = range.minReversePosition - range.maxForwardPosition;
        qDebug() << QString("  %1%: forward=%2, reverse=%3, hysteresis=%4")
                    .arg(range.rangePercent, 3)
                    .arg(range.maxForwardPosition, 6, 'f', 2)
                    .arg(range.minReversePosition, 6, 'f', 2)
                    .arg(hysteresis, 6, 'f', 2);
        
        if (range.rangePercent == 50) {
            // Для 50% ожидаем гистерезис ~4% (52 - 48)
            QVERIFY(hysteresis > 3.5 && hysteresis < 4.5);
            qDebug() << "  ✓ Hysteresis correctly detected!";
        }
    }
    
    qDebug() << "\n✓ Test passed!\n";
}

// НЕ НУЖНО добавлять QTEST_MAIN здесь - он будет в main.cpp