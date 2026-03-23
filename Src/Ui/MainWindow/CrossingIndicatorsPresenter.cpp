#include "CrossingIndicatorsPresenter.h"

CrossingIndicatorsPresenter::CrossingIndicatorsPresenter(Ui::MainWindow* ui)
    : m_ui(ui)
{
}

void CrossingIndicatorsPresenter::setIndicatorColor(QWidget* widget,
                                                    const QString& color,
                                                    const QString& border)
{
    widget->setStyleSheet(QString(
                              "background: %1;"
                              "border: 4px solid %2;"
                              "border-radius: 10px;"
                              ).arg(color, border));
}

void CrossingIndicatorsPresenter::setIndicatorByState(
    QWidget* widget,
    CrossingStatus::State state)
{
    using State = CrossingStatus::State;

    switch (state) {

    case State::Unknown:
        setIndicatorColor(widget,
                          "#A0A0A0",
                          "#505050");
        break;

    case State::Ok:
        setIndicatorColor(widget,
                          "#4E8448",
                          "#16362B");
        break;

    case State::Fail:
        setIndicatorColor(widget,
                          "#B80F0F",
                          "#510000");
        break;
    }
}

void CrossingIndicatorsPresenter::update(const CrossingStatus& cs)
{
    setIndicatorByState(
        m_ui->widget_crossingLimits_coefficientFriction_limitStatusIndicator,
        cs.frictionPercent
    );

    setIndicatorByState(
        m_ui->widget_crossingLimits_linearCharacteristic_limitStatusIndicator,
        cs.linearCharacteristic
    );

    setIndicatorByState(
        m_ui->widget_crossingLimits_range_limitStatusIndicator,
        cs.valveStroke
    );

    setIndicatorByState(
        m_ui->widget_crossingLimits_spring_limitStatusIndicator,
        cs.spring);

    setIndicatorByState(
        m_ui->widget_crossingLimits_dynamicError_limitStatusIndicator,
        cs.dynamicError
    );
}