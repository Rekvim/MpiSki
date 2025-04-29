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
    QString positionNumber; // Номер позиции
    QString manufacturer; // Производитель ЗРА
    QString valveModel; // Модель клапана
    QString serial; // Серийный номер
    QString DN; // ДУ
    QString PN; // РУ
    QString valveStroke; // Ход клапана
    QString positioner; // Модель позиционера
    QString dinamicError; // Динамическая ошибка
    QString modelDrive; // Модель привода
    quint32 safePosition; // Базовое положение привода
    quint32 driveType; // Тип привода
    QString range; // Диапазон привода
    qreal diameter; // Диаметр привода (в см)
    quint32 strokeMovement; // Движение ход клапана
    quint32 toolNumber; // Номер поворотного шкива
    qreal pulley; // Диаметр Шкива
    QString materialStuffingBoxSeal; // Материал сальникового уплотнения
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
