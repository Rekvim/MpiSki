#pragma once

#include "AbstractTestSettings.h"
#include "Src/Domain/Tests/Main/MainParams.h"

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

    Domain::Tests::Main::Params parameters() const;
private:
    Ui::MainTestSettings *ui;
    Domain::Tests::Main::Params readParamsFromUi() const;

protected:
    virtual QVector<qreal>& sequence() {
        static QVector<qreal> dummy;
        return dummy;
    }

    virtual QListWidget* sequenceListWidget() {
        return nullptr;
    }
};
