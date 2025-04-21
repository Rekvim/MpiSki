#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>

class Sensor : public QObject
{
    Q_OBJECT
public:
    explicit Sensor(QObject *parent = nullptr);
    quint16 GetRawValue() const;
    quint16 GetRawFromValue(qreal raw_value) const;
    qreal GetValue() const;
    qreal GetValueFromPercent(qreal percent);
    qreal GetPersent() const;
    QString GetPersentFormated() const;
    QString GetFormatedValue() const;
    void SetValue(quint16 value);
    void SetCoefficients(qreal k, qreal b);
    void CorrectCoefficients(qreal k);
    void SetMin();
    void SetMax();
    void SetUnit(QString unit);

private:
    quint16 value_;
    quint16 min_value_;
    quint16 max_value_;
    QString unit_ = "%.2f";
    qreal k_ = 1;
    qreal b_ = 0;
signals:

};

#endif // SENSOR_H
