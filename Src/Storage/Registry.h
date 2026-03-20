#pragma once

#include <QDate>
#include <QObject>
#include <QSettings>
#include <QColor>
#include "./Src/Ui/Setup/ValveWindow/ValveEnums.h"

struct SensorColors
{
    QString task;
    QString linear;
    QString pressure1;
    QString pressure2;
    QString pressure3;
};

struct ObjectInfo
{
    QString object = "";
    QString manufactory = "";
    QString department = "";
    QString FIO = "";
};

struct CrossingLimits {

    bool frictionEnabled = false;
    bool linearCharacteristicEnabled = false;
    bool valveStrokeEnabled = false;
    bool springEnabled = false;
    bool dynamicErrorEnabled = false;

    double frictionCoefLower = 0.0;
    double frictionCoefUpper = 0.0;

    double linearCharacteristic = 0.0;

    double valveStroke = 0.0;

    double springLower = 0.0;
    double springUpper = 0.0;
};

struct ValveInfo
{
    QString positionNumber;
    QString manufacturer;
    QString valveModel;
    QString serialNumber;
    QString DN;
    QString PN;

    QString positionerModel;
    QString dinamicErrorRecomend;

    QString solenoidValveModel;
    QString limitSwitchModel;
    QString positionSensorModel;

    StrokeMovement strokeMovement = StrokeMovement::Linear;
    QString valveStroke;
    QString driveModel;
    SafePosition safePosition = SafePosition::NormallyClosed;
    DriveType driveType = DriveType::SpringDiaphragm;

    double driveRangeLow = 0.0;
    double driveRangeHigh = 0.0;

    double driveDiameter = 0.0;

    ToolNumber toolNumber = ToolNumber::A;
    double diameterPulley = 0.0;

    StuffingBoxSeal materialStuffingBoxSeal = StuffingBoxSeal::PTFE;

    CrossingLimits crossingLimits;
};

struct OtherParameters
{
    QString date = "";
    QString safePosition = "";
    QString strokeMovement = "";
};

class Registry
{
public:
    Registry();

    // ObjectInfo
    void loadObjectInfo();
    void saveObjectInfo();

    ObjectInfo& objectInfo();
    const ObjectInfo& objectInfo() const;

    // ValveInfo
    bool loadValveInfo(const QString& position);
    void saveValveInfo();

    ValveInfo& valveInfo();
    const ValveInfo& valveInfo() const;

    // Other
    OtherParameters& otherParameters();
    const OtherParameters& otherParameters() const;

    QStringList positions();
    QString lastPosition();

    SensorColors& sensorColors();
    const SensorColors& sensorColors() const;

    bool checkObject(const QString &object);
    bool checkManufactory(const QString &manufactory);
    bool checkDepartment(const QString &department);
    bool checkPosition(const QString &position);

private:
    void setValue(const QString& key, const QVariant& val);

    QSettings m_settings;
    ObjectInfo m_objectInfo;
    ValveInfo m_valveInfo;
    SensorColors m_sensorColors;
    OtherParameters m_otherParameters;
};
