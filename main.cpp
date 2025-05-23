#include <QApplication>
#include <QTranslator>

#include "MainWindow.h"
#include "SelectTests.h"
#include "ObjectWindow.h"
#include "Registry.h"
#include "ValveWindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;

    if (qtTranslator.load("qt_ru.qm", ":/translations"))
        a.installTranslator(&qtTranslator);

    Registry registry;

    MainWindow mainWindow;

    ObjectWindow objectWindow;
    objectWindow.LoadFromReg(&registry);
    if (objectWindow.exec() == QDialog::Rejected)
        return 0;

    SelectTests selectTests;
    if (selectTests.exec() == QDialog::Rejected) {
        return 0;
    }

    ValveWindow valveWindow;
    valveWindow.SetRegistry(&registry);

    if (valveWindow.exec() == QDialog::Rejected)
        return 0;

    auto pattern = selectTests.currentPattern();
    auto cts = selectTests.getCTS();

    mainWindow.SetPatternType(pattern);
    mainWindow.SetBlockCTS(cts);
    mainWindow.SetSensorsNumber(3);
    mainWindow.SetRegistry(&registry);
    mainWindow.show();

    return a.exec();
}
