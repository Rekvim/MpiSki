#pragma once

#include "ui_MainWindow.h"
#include "Src/Storage/Telemetry.h"

class CrossingIndicatorsPresenter
{
public:
    explicit CrossingIndicatorsPresenter(Ui::MainWindow* ui);

    void update(const CrossingStatus& status);

private:
    Ui::MainWindow* m_ui;

    static void setIndicatorColor(QWidget* widget,
                                  const QString& color,
                                  const QString& border);

    static void setIndicatorByState(QWidget* widget,
                                    CrossingStatus::State state);
};
