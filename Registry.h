#ifndef REGISTRY_H
#define REGISTRY_H

#pragma once
#include <QDate>
#include <QObject>
#include <QSettings>

struct ObjectInfo
{
    QString object;
    QString manufactory;
    QString department;
    QString FIO;
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

    qreal dinamicError;
    quint32 strokeMovement;
    QString strokValve;
    QString driveModel;
    quint32 safePosition;
    quint32 driveType;
    QString driveRange;
    qreal driveDiameter;

    quint32 toolNumber;
    qreal diameterPulley;
    QString materialStuffingBoxSeal;
};

struct OtherParameters
{
    QString date;
    QString safePosition;
    QString strokeMovement;
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
