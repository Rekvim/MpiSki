#include <QtTest>
#include <QDebug>

#include "analyzers/CyclicRegulatoryAnalyzerTest.h"
#include "analyzers/StrokeTestAnalyzerTest.h"
#include "analyzers/StepTestAnalyzerTest.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Running Unit Tests for Analyzers";

    int status = 0;
    int status2 = 0;
    int status3 = 0;

    StepTestAnalyzerTest stepTestAnalyzer;
    StrokeTestAnalyzerTest strokeTestAnalyzer;
    CyclicRegulatoryAnalyzerTest сyclicRegulatoryAnalyzer;

    status = QTest::qExec(&stepTestAnalyzer, argc, argv);
    status2 = QTest::qExec(&strokeTestAnalyzer, argc, argv);
    status3 = QTest::qExec(&сyclicRegulatoryAnalyzer, argc, argv);

    if (status == 0) {
        qDebug() << "\n========================================";
        qDebug() << "ALL TESTS StepTestAnalyzerTest PASSED!";
        qDebug() << "========================================\n";
    } else {
        qDebug() << "\n========================================";
        qDebug() << "SOME TESTS StepTestAnalyzerTest FAILED!";
        qDebug() << "========================================\n";
    }
    if (status2 == 0) {
        qDebug() << "\n========================================";
        qDebug() << "ALL TESTS StrokeTestAnalyzerTest PASSED!";
        qDebug() << "========================================\n";
    } else {
        qDebug() << "\n========================================";
        qDebug() << "SOME TESTS StrokeTestAnalyzerTest FAILED!";
        qDebug() << "========================================\n";
    }
    if (status2 == 0) {
        qDebug() << "\n========================================";
        qDebug() << "ALL TESTS CyclicRegulatoryAnalyzerTest PASSED!";
        qDebug() << "========================================\n";
    } else {
        qDebug() << "\n========================================";
        qDebug() << "SOME TESTS CyclicRegulatoryAnalyzerTest FAILED!";
        qDebug() << "========================================\n";
    }

    return status;
}