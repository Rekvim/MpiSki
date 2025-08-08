#include "Registry.h"

Registry::Registry(QObject *parent)
    : QObject{parent}
    , m_settings("MPPI", "Data")
{
    m_objectInfo.object = m_settings.value("object", "").toString();
    m_objectInfo.manufactory = m_settings.value("manufactory", "").toString();
    m_objectInfo.department = m_settings.value("department", "").toString();
    m_objectInfo.FIO = m_settings.value("FIO", "").toString();
}

ObjectInfo *Registry::GetObjectInfo()
{
    return &m_objectInfo;
}

void Registry::SaveObjectInfo()
{
    m_settings.setValue("object", m_objectInfo.object);
    m_settings.setValue("manufactory", m_objectInfo.manufactory);
    m_settings.setValue("department", m_objectInfo.department);
    m_settings.setValue("FIO", m_objectInfo.FIO);
}

ValveInfo *Registry::GetValveInfo(const QString &position)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);
    m_settings.beginGroup(position);

    m_valveInfo.positionNumber = position;

    m_valveInfo.manufacturer = m_settings.value("manufacturer", "").toString();
    m_valveInfo.valveModel = m_settings.value("valveModel", "").toString();
    m_valveInfo.serialNumber = m_settings.value("serialNumber", "").toString();
    m_valveInfo.DN = m_settings.value("DN", "").toString();
    m_valveInfo.PN = m_settings.value("PN", "").toString();
    m_valveInfo.positionerModel = m_settings.value("positionerModel", "").toString();
    m_valveInfo.solenoidValveModel = m_settings.value("solenoidValveModel", "").toString();
    m_valveInfo.limitSwitchModel = m_settings.value("limitSwitchModel", "").toString();
    m_valveInfo.positionSensorModel = m_settings.value("positionSensorModel", "").toString();
    m_valveInfo.dinamicErrorRecomend = m_settings.value("dinamicErrorRecomend", "").toDouble();
    m_valveInfo.strokeMovement = m_settings.value("strokeMovement", "").toInt();
    m_valveInfo.strokValve = m_settings.value("strokValve", "").toString();
    m_valveInfo.driveModel = m_settings.value("driveModel", "").toString();
    m_valveInfo.safePosition = m_settings.value("safePosition", "").toInt();
    m_valveInfo.driveType = m_settings.value("driveType", "").toInt();
    m_valveInfo.driveRecomendRange = m_settings.value("driveRecomendRange", "").toString();
    m_valveInfo.driveDiameter = m_settings.value("driveDiameter", "").toDouble();
    m_valveInfo.toolNumber = m_settings.value("toolNumber", "").toInt();
    m_valveInfo.diameterPulley = m_settings.value("diameterPulley", "").toInt();
    m_valveInfo.materialStuffingBoxSeal = m_settings.value("materialStuffingBoxSeal", "").toString();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return &m_valveInfo;
}
ValveInfo *Registry::GetValveInfo()
{
    return &m_valveInfo;
}

void Registry::SaveValveInfo()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    m_settings.setValue("last_position", m_valveInfo.positionNumber);

    m_settings.beginGroup(m_valveInfo.positionNumber);

    m_settings.setValue("manufacturer", m_valveInfo.manufacturer);
    m_settings.setValue("valveModel", m_valveInfo.valveModel);
    m_settings.setValue("serialNumber", m_valveInfo.serialNumber);
    m_settings.setValue("DN", m_valveInfo.DN);
    m_settings.setValue("PN", m_valveInfo.PN);
    m_settings.setValue("positionerModel", m_valveInfo.positionerModel);
    m_settings.setValue("solenoidValveModel", m_valveInfo.solenoidValveModel);
    m_settings.setValue("limitSwitchModel", m_valveInfo.limitSwitchModel);
    m_settings.setValue("positionSensorModel", m_valveInfo.positionSensorModel);
    m_settings.setValue("dinamicErrorRecomend", m_valveInfo.dinamicErrorRecomend);
    m_settings.setValue("strokeMovement", m_valveInfo.strokeMovement);
    m_settings.setValue("strokValve", m_valveInfo.strokValve);
    m_settings.setValue("driveModel", m_valveInfo.driveModel);
    m_settings.setValue("safePosition", m_valveInfo.safePosition);
    m_settings.setValue("driveType", m_valveInfo.driveType);
    m_settings.setValue("driveRecomendRange", m_valveInfo.driveRecomendRange);
    m_settings.setValue("driveDiameter", m_valveInfo.driveDiameter);
    m_settings.setValue("toolNumber", m_valveInfo.toolNumber);
    m_settings.setValue("diameterPulley", m_valveInfo.diameterPulley);
    m_settings.setValue("materialStuffingBoxSeal", m_valveInfo.materialStuffingBoxSeal);

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();
}

OtherParameters *Registry::GetOtherParameters()
{
    return &m_otherParameters;
}

bool Registry::CheckObject(const QString &object)
{
    return m_settings.childGroups().contains(object);
}

bool Registry::CheckManufactory(const QString &manufactory)
{
    m_settings.beginGroup(m_objectInfo.object);

    bool result = m_settings.childGroups().contains(manufactory);

    m_settings.endGroup();

    return result;
}

bool Registry::CheckDepartment(const QString &department)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);

    bool result = m_settings.childGroups().contains(department);

    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}

bool Registry::CheckPosition(const QString &position)
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    bool result = m_settings.childGroups().contains(position);

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}

QStringList Registry::GetPositions()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    QStringList result = m_settings.childGroups();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}

QString Registry::GetLastPosition()
{
    m_settings.beginGroup(m_objectInfo.object);
    m_settings.beginGroup(m_objectInfo.manufactory);
    m_settings.beginGroup(m_objectInfo.department);

    QString result = m_settings.value("last_position", "").toString();

    m_settings.endGroup();
    m_settings.endGroup();
    m_settings.endGroup();

    return result;
}
