#include <QApplication>
#include <QTranslator>
#include <QDebug>

#include "Src/Gui/MainWindow/MainWindow.h"
#include "Src/Gui/Setup/SelectTests.h"
#include "Src/Gui/Setup/ObjectWindow.h"
#include "Src/Storage/Registry.h"
#include "Src/Gui/Setup/ValveWindow/ValveWindow.h"
#include "Src/Domain/DeviceProfile.h"

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

    const Domain::DeviceProfile profile = selectTests.profile();

    ValveWindow valveWindow;
    valveWindow.setRegistry(&registry);
    valveWindow.setProfile(profile);

    if (valveWindow.exec() == QDialog::Rejected)
        return 0;

    MainWindow mainWindow;
    mainWindow.setProfile(profile);
    mainWindow.setRegistry(&registry);
    mainWindow.show();
    QTimer::singleShot(0, &mainWindow, [&]{ mainWindow.showMaximized(); });
    return a.exec();
}
