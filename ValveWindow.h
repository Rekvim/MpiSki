#ifndef VALVE_WINDOW_H
#define VALVE_WINDOW_H

#pragma once
#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include "Registry.h"

namespace Ui {
class ValveWindow;
}

class ValveWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ValveWindow(QWidget *parent = nullptr);
    ~ValveWindow();
    void SetRegistry(Registry *registry);

private:
    Ui::ValveWindow *ui;
    Registry *m_registry;
    ValveInfo *m_valveInfo;
    void SaveValveInfo();
    const QString m_manualInput = "Ручной ввод";
    static constexpr qreal m_diameter[] = {50.0, 86.0, 108.0, 125.0};
    QJsonObject m_valveDataObj;

private slots:
    void PositionChanged(const QString &position);
    void ButtonClick();
    void StrokeChanged(quint16 n);
    void ToolChanged(quint16 n);
    void DiameterChanged(qreal value);
    void Clear();
};

#endif // VALVE_WINDOW_H
