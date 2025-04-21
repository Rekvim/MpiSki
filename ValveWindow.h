#ifndef VALVE_WINDOW_H
#define VALVE_WINDOW_H

#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
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
    void on_comboBox_DN_currentIndexChanged(const QString &selectedDN);
private:
    Ui::ValveWindow *ui;
    Registry *registry_;
    ValveInfo *valve_info_;
    void SaveValveInfo();
    const QString manual_input_ = "Ручной ввод";
    static constexpr qreal diameter_[] = {50.0, 86.0, 108.0, 125.0};
    QJsonObject m_valveDataObj;
private slots:
    void loadDNValues();
    void PositionChanged(const QString &position);
    void ButtonClick();
    void StrokeChanged(quint16 n);
    void ToolChanged(quint16 n);
    void DiameterChanged(qreal value);
    void Clear();
};

#endif // VALVE_WINDOW_H
