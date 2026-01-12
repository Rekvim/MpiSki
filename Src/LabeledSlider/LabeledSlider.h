#ifndef LABELEDSLIDER_H
#define LABELEDSLIDER_H

#pragma once
#include <QSlider>
#include <QMap>

class LabeledSlider : public QSlider
{
    Q_OBJECT
public:
    explicit LabeledSlider(QWidget* parent = nullptr);

    void setTickLabels(const QMap<int, QString>& labels);
    void setLabelOffset(int px);
    void setTickLength(int px);
    void setTickGap(int px);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* e) override;
    bool event(QEvent* e) override;

private:
    int labelsMaxTextWidth() const;
    int leftAreaWidth() const;
    void updateWidthConstraints();     // <-- ВАЖНО

private:
    QMap<int, QString> m_labels;
    int m_labelOffset = 6;
    int m_tickLen     = 10;
    int m_tickGap = 4;
};

#endif // LABELEDSLIDER_H
