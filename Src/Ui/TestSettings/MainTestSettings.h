#pragma once

#include "AbstractTestSettings.h"
#include "Src/Domain/Tests/Main/MainTestParams.h"

namespace Ui {
class MainTestSettings;
}

class MainTestSettings :public AbstractTestSettings
{
    Q_OBJECT

public:
    explicit MainTestSettings(QWidget *parent = nullptr);
    ~MainTestSettings();

    void applyValveInfo(const ValveInfo& info) override;
    void applyPattern(SelectTests::PatternType pattern) override;

    MainTestParams parameters() const;
private:
    Ui::MainTestSettings *ui;
    MainTestParams readParamsFromUi() const;

protected:
    virtual QVector<qreal>& sequence() {
        static QVector<qreal> dummy;
        return dummy;
    }

    virtual QListWidget* sequenceListWidget() {
        return nullptr;
    }
};
