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
    Src/ReportBuilders/ReportSaver.cpp \
    Src/Tests/CyclicTestPositioner.cpp \
    Src/Tests/CyclicTestSolenoid.cpp \
    Src/Tests/MainTest.cpp \
    Src/Tests/OptionTest.cpp \
    Src/Tests/StepTest.cpp \
    Src/Tests/StrokeTest.cpp \
    Src/Tests/Test.cpp \
    Src/Uart/Uart.cpp \
    Src/Uart/UartMessage.cpp \
    Src/Uart/UartReader.cpp \
    Src/ValidatorFactory/ValidatorFactory.cpp \
    main.cpp \
    MainWindow.cpp \
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
    ValveWindow.cpp

HEADERS += \
    CyclicTestSettings.h \
    MainWindow.h \
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
    Src/ReportBuilders/ReportSaver.h \
    Src/Tests/CyclicTestPositioner.h \
    Src/Tests/CyclicTestSolenoid.h \
    Src/Tests/MainTest.h \
    Src/Tests/OptionTest.h \
    Src/Tests/StepTest.h \
    Src/Tests/StrokeTest.h \
    Src/Tests/Test.h \
    Src/Uart/Uart.h \
    Src/Uart/UartMessage.h \
    Src/Uart/UartReader.h \
    Src/ValidatorFactory/RegexPatterns.h \
    Src/ValidatorFactory/ValidatorFactory.h \
    StepTestSettings.h \
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
