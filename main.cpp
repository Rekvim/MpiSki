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
    if (selectTests.exec() == QDialog::Rejected)
        return 0;

    auto selectedPattern = selectTests.currentPattern();

    ValveWindow valveWindow;
    valveWindow.SetRegistry(&registry);
    valveWindow.SetPatternType(selectedPattern);

    if (valveWindow.exec() == QDialog::Rejected)
        return 0;

    auto cts = selectTests.getCTS();

    mainWindow.SetPatternType(selectedPattern);
    // mainWindow.SetSensorsNumber(3);
    mainWindow.SetBlockCTS(cts);
    mainWindow.SetRegistry(&registry);
    mainWindow.show();

    return a.exec();
}
