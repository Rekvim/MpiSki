QT += core gui widgets
QT += serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QXLSX_PARENTPATH=./
QXLSX_HEADERPATH=./header/
QXLSX_SOURCEPATH=./source/
include(./QXlsx.pri)

SOURCES += \
    CyclicTestSettings.cpp \
    Src/Tests/CyclicTestSolenoid.cpp \
    main.cpp \
    MainWindow.cpp \
    FileSaver.cpp \
    MainTestSettings.cpp \
    Mpi.cpp \
    MpiSettings.cpp \
    MyChart.cpp \
    ObjectWindow.cpp \
    OtherTestSettings.cpp \
    Program.cpp \
    Registry.cpp \
    SelectTests.cpp \
    Sensor.cpp \
    StepTestSettings.cpp \
    Test.cpp \
    Uart.cpp \
    UartReader.cpp \
    ValveWindow.cpp

HEADERS += \
    CyclicTestSettings.h \
    MainWindow.h \
    FileSaver.h \
    MainTestSettings.h \
    Mpi.h \
    MpiSettings.h \
    MyChart.h \
    ObjectWindow.h \
    OtherTestSettings.h \
    Program.h \
    Registry.h \
    SelectTests.h \
    Sensor.h \
    Src/Tests/CyclicTestSolenoid.h \
    StepTestSettings.h \
    Test.h \
    Uart.h \
    UartReader.h \
    ValveWindow.h \

FORMS += \
    CyclicTestSettings.ui \
    MainTestSettings.ui \
    MainWindow.ui \
    ObjectWindow.ui \
    OtherTestSettings.ui \
    SelectTests.ui \
    StepTestSettings.ui \
    ValveWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



RESOURCES += \
    excel.qrc \
    translations.qrc
