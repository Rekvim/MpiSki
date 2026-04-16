#pragma once

#include <QObject>

class CyclicRegulatoryAnalyzerTest : public QObject
{
    Q_OBJECT

private slots:
    void testWithManyPoints();
    void testForwardMax();
    void testBackwardMin();
    void testOnlyForward();
    void testExtremumStability();
};