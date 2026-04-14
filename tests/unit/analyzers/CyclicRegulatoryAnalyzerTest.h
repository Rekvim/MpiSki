#ifndef CYCLICREGULATORYANALYZERTEST_H
#define CYCLICREGULATORYANALYZERTEST_H

#include <QtTest>
#include <QDebug>

// Не включайте здесь cpp файлы!
class CyclicRegulatoryAnalyzerTest : public QObject
{
    Q_OBJECT

private slots:
    void testConfigureWithFullSequence();
    void testIdealMotion();
    void testHysteresisDetection();
};

#endif