#include <QApplication>
#include <QTranslator>
#include <QDebug>

#include "Src/Gui/MainWindow/MainWindow.h"
#include "Src/Gui/Setup/SelectTests.h"
#include "Src/Gui/Setup/ObjectWindow.h"
#include "Src/Storage/Registry.h"
#include "Src/Gui/Setup/ValveWindow/ValveWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;

    if (qtTranslator.load("qt_ru.qm", ":/translations"))
        a.installTranslator(&qtTranslator);

    Registry registry;

    ObjectWindow objectWindow;
    objectWindow.LoadFromReg(&registry);
    if (objectWindow.exec() == QDialog::Rejected)
        return 0;

    SelectTests selectTests;
    if (selectTests.exec() == QDialog::Rejected)
        return 0;

    auto selectedPattern = selectTests.currentPattern();

    ValveWindow valveWindow;
    valveWindow.setRegistry(&registry);
    valveWindow.setPatternType(selectedPattern);

    if (valveWindow.exec() == QDialog::Rejected)
        return 0;

    MainWindow mainWindow;
    mainWindow.setPatternType(selectedPattern);
    mainWindow.setRegistry(&registry);
    mainWindow.show();
    QTimer::singleShot(0, &mainWindow, [&]{ mainWindow.showMaximized(); });
    return a.exec();
}
