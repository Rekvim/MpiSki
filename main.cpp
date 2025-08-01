#include <QApplication>
#include <QTranslator>
#include <QDebug>

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

    qRegisterMetaType<TelemetryStore>("TelemetryStore");

    Registry registry;

    MainWindow mainWindow(registry);

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
    mainWindow.SetBlockCTS(cts);
    mainWindow.initializeFromRegistry(&registry);
    mainWindow.show();
    mainWindow.showMaximized();
    return a.exec();
}
