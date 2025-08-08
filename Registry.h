#ifndef REGISTRY_H
#define REGISTRY_H

#pragma once
#include <QDate>
#include <QObject>
#include <QSettings>

struct ObjectInfo
{
    QString object = "";
    QString manufactory = "";
    QString department = "";
    QString FIO = "";
};

struct ValveInfo
{
    QString positionNumber = "";

    QString manufacturer = "";
    QString valveModel = "";
    QString serialNumber = "";
    QString DN = "";
    QString PN = "";
    QString positionerModel = "";
    QString positionerType = "";
    qreal dinamicErrorRecomend = 0.0;

    QString solenoidValveModel = "";
    QString limitSwitchModel = "";
    QString positionSensorModel = "";

    quint32 strokeMovement = 0;
    QString strokValve = "";
    QString driveModel = "";
    quint32 safePosition = 0;
    quint32 driveType = 0;
    QString driveRecomendRange = "";
    qreal driveDiameter = 0.0;

    quint32 toolNumber = 0;
    qreal diameterPulley = 0.0;
    QString materialStuffingBoxSeal = "";
};

struct TestTelemetryData {
    // double dinamicReal = 0.0;
    // double dinamicRecomend = 0.0;
    // double dinamicIpReal = 0.0;
    // double dinamicIpRecomend = 0.0;

    // double strokeReal = 0.0;
    // double strokeRecomend = 0.0;

    // double rangeReal = 0.0;
    // double rangeRecomend = 0.0;
    // double rangePressure = 0.0;

    // double frictionPercent = 0.0;
    // double friction = 0.0;

    // double strokeTest_timeForward = 0.0;
    // double strokeTest_timeBackward = 0.0;

    // QString cyclicTest_sequence = "";
    // quint32 cyclicTest_cycles = 0;
    // double cyclicTest_totalTime = 0.0;

    double supplyPressure = 0.0;
};

struct OtherParameters
{
    QString date = "";
    QString safePosition = "";
    QString strokeMovement = "";
};

class Registry : public QObject
{
    Q_OBJECT
public:
    explicit Registry(QObject *parent = nullptr);
    ObjectInfo *GetObjectInfo();
    void SaveObjectInfo();
    ValveInfo *GetValveInfo(const QString &position);
    ValveInfo *GetValveInfo();
    void SaveValveInfo();
    OtherParameters *GetOtherParameters();
    bool CheckObject(const QString &object);
    bool CheckManufactory(const QString &manufactory);
    bool CheckDepartment(const QString &department);
    bool CheckPosition(const QString &position);
    QStringList GetPositions();
    QString GetLastPosition();

private:
    QSettings m_settings;
    ObjectInfo m_objectInfo;
    ValveInfo m_valveInfo;
    OtherParameters m_otherParameters;
signals:
};

#endif // REGISTRY_H
