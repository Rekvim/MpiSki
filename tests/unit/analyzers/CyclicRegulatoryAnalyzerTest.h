#pragma once

#include <QObject>

class CyclicRegulatoryAnalyzerTest : public QObject
{
    Q_OBJECT

private slots:
    void testWithManyPoints();
    void testForwardMax();
    void testBackwardMax();
    void testOnlyForward();
    void testExtremumStability();
};