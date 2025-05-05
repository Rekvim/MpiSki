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
    QString serial;

    QString manufacturer;
    QString valveModel;

    QString DN;
    QString PN;

    QString valveStroke;
    QString positioner;
    QString dinamicError;
    QString modelDrive;
    quint32 safePosition;
    quint32 driveType;
    QString range;
    qreal diameter;
    quint32 strokeMovement;
    quint32 toolNumber;
    qreal pulley;
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
