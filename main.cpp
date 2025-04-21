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

    MainWindow main_window;

    ObjectWindow object_window; // Окно данных об объекте
    object_window.LoadFromReg(&registry);
    if (object_window.exec() == QDialog::Rejected)
        return 0;

    SelectTests select_tests;
    if (select_tests.exec() == QDialog::Rejected) {
        return 0;
    }

    ValveWindow valve_window; // Окно данных об объекте
    valve_window.SetRegistry(&registry);

    if (valve_window.exec() == QDialog::Rejected)
        return 0;

    SelectTests::BlockCTS blockCTS = select_tests.getCTS();
    main_window.SetBlockCTS(blockCTS);
    main_window.SetSensorsNumber(3); // !!!
    main_window.SetRegistry(&registry);
    main_window.show();

    return a.exec();
}
