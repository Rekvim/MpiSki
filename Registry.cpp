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

    m_valveInfo.materialStuffingBoxSeal = m_settings.value("materialStuffingBoxSeal", "").toString();
    m_valveInfo.manufacturer = m_settings.value("manufacturer", "").toString();
    m_valveInfo.valveStroke = m_settings.value("valveStroke", "").toString();
    m_valveInfo.valveModel = m_settings.value("valveModel", "").toString();
    m_valveInfo.positioner = m_settings.value("positioner", "").toString();
    m_valveInfo.modelDrive = m_settings.value("modelDrive", "").toString();
    m_valveInfo.serial = m_settings.value("serial", "").toString();

    m_valveInfo.range = m_settings.value("range", "").toString();
    m_valveInfo.DN = m_settings.value("DN", "").toString();
    m_valveInfo.PN = m_settings.value("PN", "").toString();

    m_valveInfo.strokeMovement = m_settings.value("strokeMovement", "").toInt();
    m_valveInfo.safePosition = m_settings.value("safePosition", "").toInt();
    m_valveInfo.toolNumber = m_settings.value("toolNumber", "").toInt();
    m_valveInfo.driveType = m_settings.value("driveType", "").toInt();
    m_valveInfo.pulley = m_settings.value("pulley", "").toInt();

    m_valveInfo.dinamicError = m_settings.value("dinamicError", "").toDouble();
    m_valveInfo.diameter = m_settings.value("diameter", "").toDouble();


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
    m_settings.setValue("serial", m_valveInfo.serial);
    m_settings.setValue("DN", m_valveInfo.DN);
    m_settings.setValue("PN", m_valveInfo.PN);
    m_settings.setValue("stroke", m_valveInfo.valveStroke);
    m_settings.setValue("positioner", m_valveInfo.positioner);
    m_settings.setValue("dinamicError", m_valveInfo.dinamicError);
    m_settings.setValue("modelDrive", m_valveInfo.modelDrive);
    m_settings.setValue("range", m_valveInfo.range);
    m_settings.setValue("materialStuffingBoxSeal", m_valveInfo.materialStuffingBoxSeal);
    m_settings.setValue("diameter", m_valveInfo.diameter);
    m_settings.setValue("safePosition", m_valveInfo.safePosition);
    m_settings.setValue("type_of_drive", m_valveInfo.driveType);
    m_settings.setValue("strokeMovement", m_valveInfo.strokeMovement);
    m_settings.setValue("toolNumber", m_valveInfo.toolNumber);
    m_settings.setValue("pulley", m_valveInfo.pulley);

    // m_settings.setValue("Корпус", m_valveInfo.corpus);
    // m_settings.setValue("Крышка", m_valveInfo.cap);
    // m_settings.setValue("Материал седла", m_valveInfo.saddle_materials);
    // m_settings.setValue("CV", m_valveInfo.saddle_cv);
    // m_settings.setValue("Шар", m_valveInfo.ball);
    // m_settings.setValue("Диск", m_valveInfo.disk);
    // m_settings.setValue("Плунжер", m_valveInfo.plunger);
    // m_settings.setValue("Вал", m_valveInfo.shaft);
    // m_settings.setValue("Шток", m_valveInfo.stock);
    // m_settings.setValue("Направляющаяя втулка", m_valveInfo.guide_sleeve);

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
