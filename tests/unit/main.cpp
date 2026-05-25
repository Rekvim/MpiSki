#include <QtTest>
#include <QDebug>

#include "analyzers/CyclicRegulatoryAnalyzerTest.h"
#include "analyzers/MainTestAnalyzerTest.h"
#include "analyzers/StrokeTestAnalyzerTest.h"
#include "analyzers/StepTestAnalyzerTest.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Running Unit Tests for Analyzers";

    int status  = 0;
    int status2 = 0;
    int status3 = 0;
    int status4 = 0;

    StepTestAnalyzerTest         stepTestAnalyzer;
    StrokeTestAnalyzerTest       strokeTestAnalyzer;
    CyclicRegulatoryAnalyzerTest сyclicRegulatoryAnalyzer;
    MainTestAnalyzerTest         mainTestAnalyzer;

    status  = QTest::qExec(&stepTestAnalyzer,        argc, argv);
    status2 = QTest::qExec(&strokeTestAnalyzer,      argc, argv);
    status3 = QTest::qExec(&сyclicRegulatoryAnalyzer, argc, argv);
    status4 = QTest::qExec(&mainTestAnalyzer,        argc, argv);

    auto printBanner = [](int s, const char* name) {
        if (s == 0) {
            qDebug() << "\n========================================";
            qDebug() << "ALL TESTS" << name << "PASSED!";
            qDebug() << "========================================\n";
        } else {
            qDebug() << "\n========================================";
            qDebug() << "SOME TESTS" << name << "FAILED!";
            qDebug() << "========================================\n";
        }
    };

    printBanner(status,  "StepTestAnalyzerTest");
    printBanner(status2, "StrokeTestAnalyzerTest");
    printBanner(status3, "CyclicRegulatoryAnalyzerTest");
    printBanner(status4, "MainTestAnalyzerTest");

    return status | status2 | status3 | status4;
}