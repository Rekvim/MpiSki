#pragma once

#include <QtTest>

class CyclicRegulatoryAnalyzerTest : public QObject
{
    Q_OBJECT

private slots:
    void testWithManyPoints();
    void testForwardMax();
    void testReverseMin();
    void testOnlyForward();
    void testExtremumStability();
};