#include <QtTest>
#include <QDebug>

// Подключаем заголовок теста
#include "analyzers/CyclicRegulatoryAnalyzerTest.h"

int main(int argc, char *argv[])
{
    // Используем QCoreApplication вместо QApplication
    QCoreApplication app(argc, argv);

    qDebug() << "\n========================================";
    qDebug() << "Running Unit Tests for Analyzers";
    qDebug() << "========================================\n";

    int status = 0;

    // Запускаем тесты
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