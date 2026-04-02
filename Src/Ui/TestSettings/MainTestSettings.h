#pragma once

#include <QDialog>
#include <QMetaType>
#include "AbstractTestSettings.h"
#include "Src/Domain/TestParams/MainTestParams.h"

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

    MainTestParams parameters() const { return m_params; }
private:
    Ui::MainTestSettings *ui;
    MainTestParams m_params;

    void readUi();
private slots:
    void accept() override;

protected:
    virtual QVector<qreal>& sequence() {
        static QVector<qreal> dummy;
        return dummy;
    }

    virtual QListWidget* sequenceListWidget() {
        return nullptr;
    }
};
