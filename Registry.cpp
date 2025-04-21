#include "Registry.h"

Registry::Registry(QObject *parent)
    : QObject{parent}
    , settings_("MPPI", "Data")
{
    object_info_.object = settings_.value("object", "").toString();
    object_info_.manufactory = settings_.value("manufactory", "").toString();
    object_info_.department = settings_.value("department", "").toString();
    object_info_.FIO = settings_.value("FIO", "").toString();
}

ObjectInfo *Registry::GetObjectInfo()
{
    return &object_info_;
}

void Registry::SaveObjectInfo()
{
    settings_.setValue("object", object_info_.object);
    settings_.setValue("manufactory", object_info_.manufactory);
    settings_.setValue("department", object_info_.department);
    settings_.setValue("FIO", object_info_.FIO);
}

ValveInfo *Registry::GetValveInfo(const QString &position)
{

    settings_.beginGroup(object_info_.object);
    settings_.beginGroup(object_info_.manufactory);
    settings_.beginGroup(object_info_.department);
    settings_.beginGroup(position);

    valve_info_.position = position;
    valve_info_.manufacturer = settings_.value("manufacturer", "").toString();
    valve_info_.model = settings_.value("model", "").toString();
    valve_info_.serial = settings_.value("serial", "").toString();
    valve_info_.DN = settings_.value("DN", "").toString();
    valve_info_.PN = settings_.value("PN", "").toString();
    valve_info_.stroke = settings_.value("stroke", "").toString();
    valve_info_.positioner = settings_.value("positioner", "").toString();
    valve_info_.dinamic_error = settings_.value("dinamic_error", "").toString();
    valve_info_.model_drive = settings_.value("model_drive", "").toString();
    valve_info_.range = settings_.value("range", "").toString();
    valve_info_.material = settings_.value("material", "").toString();
    valve_info_.diameter = settings_.value("diameter", "").toDouble();
    valve_info_.normal_position = settings_.value("normal_position", "").toInt();
    valve_info_.drive_type = settings_.value("type_of_drive", "").toInt();
    valve_info_.stroke_movement = settings_.value("stroke_movement", "").toInt();
    valve_info_.tool_number = settings_.value("tool_number", "").toInt();
    valve_info_.pulley = settings_.value("pulley", "").toInt();

    valve_info_.corpus = settings_.value("Корпус", "").toString();
    valve_info_.cap = settings_.value("Крышка", "").toString();
    valve_info_.saddle_materials = settings_.value("Материал седла", "").toString();
    valve_info_.saddle_cv = settings_.value("CV", "").toString();
    valve_info_.ball = settings_.value("Шар", "").toString();
    valve_info_.disk = settings_.value("Диск", "").toString();
    valve_info_.plunger = settings_.value("Плунжер", "").toString();
    valve_info_.shaft = settings_.value("Вал", "").toString();
    valve_info_.stock = settings_.value("Шток", "").toString();
    valve_info_.guide_sleeve = settings_.value("Направляющаяя втулка", "").toString();

    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();

    return &valve_info_;
}

ValveInfo *Registry::GetValveInfo()
{
    return &valve_info_;
}

void Registry::SaveValveInfo()
{
    settings_.beginGroup(object_info_.object);
    settings_.beginGroup(object_info_.manufactory);
    settings_.beginGroup(object_info_.department);

    settings_.setValue("last_position", valve_info_.position);

    settings_.beginGroup(valve_info_.position);

    settings_.setValue("manufacturer", valve_info_.manufacturer);
    settings_.setValue("model", valve_info_.model);
    settings_.setValue("serial", valve_info_.serial);
    settings_.setValue("DN", valve_info_.DN);
    settings_.setValue("PN", valve_info_.PN);
    settings_.setValue("stroke", valve_info_.stroke);
    settings_.setValue("positioner", valve_info_.positioner);
    settings_.setValue("dinamic_error", valve_info_.dinamic_error);
    settings_.setValue("model_drive", valve_info_.model_drive);
    settings_.setValue("range", valve_info_.range);
    settings_.setValue("material", valve_info_.material);
    settings_.setValue("diameter", valve_info_.diameter);
    settings_.setValue("normal_position", valve_info_.normal_position);
    settings_.setValue("type_of_drive", valve_info_.drive_type);
    settings_.setValue("stroke_movement", valve_info_.stroke_movement);
    settings_.setValue("tool_number", valve_info_.tool_number);
    settings_.setValue("pulley", valve_info_.pulley);

    settings_.setValue("Корпус", valve_info_.corpus);
    settings_.setValue("Крышка", valve_info_.cap);
    settings_.setValue("Материал седла", valve_info_.saddle_materials);
    settings_.setValue("CV", valve_info_.saddle_cv);
    settings_.setValue("Шар", valve_info_.ball);
    settings_.setValue("Диск", valve_info_.disk);
    settings_.setValue("Плунжер", valve_info_.plunger);
    settings_.setValue("Вал", valve_info_.shaft);
    settings_.setValue("Шток", valve_info_.stock);
    settings_.setValue("Направляющаяя втулка", valve_info_.guide_sleeve);

    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();
}

OtherParameters *Registry::GetOtherParameters()
{
    return &other_parameters_;
}

bool Registry::CheckObject(const QString &object)
{
    return settings_.childGroups().contains(object);
}

bool Registry::CheckManufactory(const QString &manufactory)
{
    settings_.beginGroup(object_info_.object);

    bool result = settings_.childGroups().contains(manufactory);

    settings_.endGroup();

    return result;
}

bool Registry::CheckDepartment(const QString &department)
{
    settings_.beginGroup(object_info_.object);
    settings_.beginGroup(object_info_.manufactory);

    bool result = settings_.childGroups().contains(department);

    settings_.endGroup();
    settings_.endGroup();

    return result;
}

bool Registry::CheckPosition(const QString &position)
{
    settings_.beginGroup(object_info_.object);
    settings_.beginGroup(object_info_.manufactory);
    settings_.beginGroup(object_info_.department);

    bool result = settings_.childGroups().contains(position);

    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();

    return result;
}

QStringList Registry::GetPositions()
{
    settings_.beginGroup(object_info_.object);
    settings_.beginGroup(object_info_.manufactory);
    settings_.beginGroup(object_info_.department);

    QStringList result = settings_.childGroups();

    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();

    return result;
}

QString Registry::GetLastPosition()
{
    settings_.beginGroup(object_info_.object);
    settings_.beginGroup(object_info_.manufactory);
    settings_.beginGroup(object_info_.department);

    QString result = settings_.value("last_position", "").toString();

    settings_.endGroup();
    settings_.endGroup();
    settings_.endGroup();

    return result;
}
