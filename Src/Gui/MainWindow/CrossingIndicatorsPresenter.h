#pragma once

#include "Storage/Telemetry.h"
#include <QWidget>

namespace Ui {
class MainWindow;
}

class CrossingIndicatorsPresenter
{
public:
    explicit CrossingIndicatorsPresenter(Ui::MainWindow* ui) : m_ui(ui) { }

    void update(const CrossingStatus& status);

private:
    Ui::MainWindow* m_ui;

    static void setIndicatorColor(QWidget* widget,
                                  const QString& color,
                                  const QString& border);

    static void setIndicatorByState(QWidget* widget,
                                    CrossingStatus::State state);
};
