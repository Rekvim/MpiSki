#ifndef REGISTRY_H
#define REGISTRY_H

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
    // Данные о ЗРА
    QString position; // Номер позиции
    QString manufacturer; // Производитель ЗРА
    QString model; // Модель клапана
    QString serial; // Серийный номер
    QString DN; // ДУ
    QString PN; // РУ
    QString stroke; // Ход клапана
    QString positioner; // Модель позиционера
    QString dinamic_error; // Динамическая ошибка
    QString model_drive; // Модель привода
    quint32 normal_position; // Базовое положение привода
    quint32 drive_type; // Тип привода
    QString range; // Диапазон привода
    qreal diameter; // Диаметр привода (в см)
    quint32 stroke_movement; // Движение ход клапана
    quint32 tool_number; // Номер поворотного шкива
    qreal pulley; // Диаметр Шкива
    QString material; // Материал сальникового уплотнения

    // // Материалы деталей
    // QString corpus; // Корпус
    // QString cap; // Крышка
    // QString saddle_materials; // Материал седла
    // QString saddle_cv; // CV
    // QString ball; // Шар
    // QString disk; // Диск
    // QString plunger; // Плунжер
    // QString shaft; // Вал
    // QString stock; // Шток
    // QString guide_sleeve; // Направляющаяя втулка
};

struct OtherParameters
{
    QString data;
    QString normal_position;
    QString movement;
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
    QSettings settings_;
    ObjectInfo object_info_;
    ValveInfo valve_info_;
    OtherParameters other_parameters_;
signals:
};

#endif // REGISTRY_H
