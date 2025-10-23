#ifndef VALVE_WINDOW_H
#define VALVE_WINDOW_H

#pragma once
#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include "Registry.h"
#include "SelectTests.h"

namespace Ui {
class ValveWindow;
}

class ValveWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ValveWindow(QWidget *parent = nullptr);
    ~ValveWindow();
    void setRegistry(Registry *registry);
    void setPatternType(SelectTests::PatternType pattern);

private:
    Ui::ValveWindow *ui;
    Registry *m_registry;
    ValveInfo *m_valveInfo;

    const QString m_manualInput = "Ручной ввод";
    QList<QString> m_diameter = {"50.0", "86.0", "108.0", "125.0"};
    QJsonObject m_valveDataObj;

    SelectTests::PatternType m_patternType = SelectTests::Pattern_None;

    void saveValveInfo();
    void applyPatternVisibility();


private slots:
    void onPositionerTypeChanged(quint8 index);
    void positionChanged(const QString &position);
    void strokeChanged(quint16 n);
    void toolChanged(quint16 n);
    void diameterChanged(const QString &text);
    void on_pushButton_netWindow_clicked();
    void on_pushButton_clear_clicked();
};

#endif // VALVE_WINDOW_H
