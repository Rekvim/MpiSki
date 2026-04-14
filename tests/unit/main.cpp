#include <QtTest>
#include <QDebug>

#include "analyzers/CyclicRegulatoryAnalyzerTest.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "\n";
    qDebug() << "\n";
    qDebug() << "Running Unit Tests for Analyzers";
    qDebug() << "\n";
    qDebug() << "\n";

    int status = 0;

    CyclicRegulatoryAnalyzerTest test;
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