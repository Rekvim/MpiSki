#ifndef REGISTRY_H
#define REGISTRY_H

#pragma once
#include <QDate>
#include <QObject>
#include <QSettings>

struct CrossingLimits {

    bool frictionEnabled = false;
    bool linearCharacteristicEnabled = false;
    bool rangeEnabled = false;
    bool springEnabled = false;
    bool dynamicErrorEnabled = false;

    double frictionCoefLowerLimit = 0.0;
    double frictionCoefUpperLimit = 0.0;

    double linearCharacteristicLowerLimit = 0.0;

    double rangeUpperLimit  = 0.0;

    double springLowerLimit = 0.0;
    double springUpperLimit = 0.0;
};

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
    double driveRangeLow  = 0.0;
    double driveRangeHigh = 0.0;

    qreal driveDiameter = 0.0;

    quint32 toolNumber = 0;
    qreal diameterPulley = 0.0;
    QString materialStuffingBoxSeal = "";

    CrossingLimits crossingLimits;
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
    void saveObjectInfo();
    void saveValveInfo();

    ValveInfo *getValveInfo(const QString &position);
    ValveInfo *getValveInfo();

    OtherParameters *getOtherParameters();
    ObjectInfo *getObjectInfo();

    QStringList getPositions();
    QString getLastPosition();

    bool checkObject(const QString &object);
    bool checkManufactory(const QString &manufactory);
    bool checkDepartment(const QString &department);
    bool checkPosition(const QString &position);

private:
    QSettings m_settings;
    ObjectInfo m_objectInfo;
    ValveInfo m_valveInfo;
    OtherParameters m_otherParameters;
signals:
};

#endif // REGISTRY_H
