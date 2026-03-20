#pragma once

#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include "./Src/Storage/Registry.h"
#include "../SelectTests.h"
#include "./Src/Storage/AppSettings.h"

class Mapper;

namespace Ui {
class ValveWindow;
}

class ValveWindow : public QDialog
{
    Q_OBJECT

    friend class Mapper;

public:
    explicit ValveWindow(QWidget *parent = nullptr);
    ~ValveWindow() = default;

    void setRegistry(Registry *registry);
    void setPatternType(SelectTests::PatternType pattern);

private:
    Ui::ValveWindow *ui;

    Registry *m_registry;
    AppSettings m_settings;
    ValveInfo m_local;

    void onDriveTypeChanged(int index);

    void loadLinearRange();
    void saveLinearRange();

    void updateManufacturerUi();
    void setupWindowGeometry();
    void bindTabShortcut(int key, QWidget* tab);
    void setupTabs();
    void setupValidators();
    void setupConnections();
    void setupInitialUi();

    static constexpr const char* kManualInput = "Ручной ввод";
    QList<QString> m_diameter = {
        QStringLiteral("50.0"),
        QStringLiteral("86.0"),
        QStringLiteral("108.0"),
        QStringLiteral("125.0")
    };

    SelectTests::PatternType m_patternType = SelectTests::Pattern_None;

    void saveValveInfo();
    void applyPatternVisibility();
    void applyFrictionLimitsFromStuffingBoxSeal();

private slots:
    void onPositionerTypeChanged(quint8 index);
    void positionChanged(const QString &position);
    void strokeChanged(quint16 n);
    void toolChanged(quint16 n);
    void on_pushButton_netWindow_clicked();
    void on_pushButton_clear_clicked();
};
