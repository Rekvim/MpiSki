#include <QtTest>
#include <QDebug>

#include "analyzers/CyclicRegulatoryAnalyzerTest.h"
#include "analyzers/StrokeTestAnalyzerTest.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Running Unit Tests for Analyzers";

    int status = 0;

    CyclicRegulatoryAnalyzerTest test;
    // StrokeTestAnalyzerTest test;

    status = QTest::qExec(&test, argc, argv);

    if (status == 0) {
        qDebug() << "\n========================================";
        qDebug() << "ALL TESTS PASSED!";
        qDebug() << "========================================\n";
    } else {
        qDebug() << "\n========================================";
        qDebug() << "SOME TESTS FAILED!";
        qDebug() << "========================================\n";
    }

    return status;
}