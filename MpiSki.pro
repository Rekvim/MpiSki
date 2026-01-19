QT += core gui widgets
QT += serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 
# static release

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QXLSX_PARENTPATH=./
QXLSX_HEADERPATH=./header/
QXLSX_SOURCEPATH=./source/
include(./QXlsx.pri)

INCLUDEPATH += $$PWD/Src/CustomChart

SOURCES += \
    CyclicTestSettings.cpp \
    Src/CustomChart/MyChart.cpp \
    Src/CustomChart/MySeries.cpp \
    Src/LabeledSlider/LabeledSlider.cpp \
    Src/Mpi/Mpi.cpp \
    Src/Mpi/MpiSettings.cpp \
    Src/ReportBuilders/ReportBuilder_B_CVT.cpp \
    Src/ReportBuilders/ReportBuilder_B_SACVT.cpp \
    Src/ReportBuilders/ReportBuilder_C_CVT.cpp \
    Src/ReportBuilders/ReportBuilder_C_SACVT.cpp \
    Src/ReportBuilders/ReportBuilder_C_SOVT.cpp \
    Src/ReportBuilders/ReportSaver.cpp \
    Src/Runners/BaseRunner.cpp \
    Src/Runners/CyclicRegulatoryRunner.cpp \
    Src/Runners/CyclicShutoffRunner.cpp \
    Src/Runners/MainTestRunner.cpp \
    Src/Runners/OptionResolutionRunner.cpp \
    Src/Runners/OptionResponseRunner.cpp \
    Src/Runners/RunnerFactory.cpp \
    Src/Runners/StepTestRunner.cpp \
    Src/Runners/StrokeTestRunner.cpp \
    Src/Telemetry/TelemetryStore.cpp \
    Src/Tests/CyclicTestPositioner.cpp \
    Src/Tests/CyclicTests.cpp \
    Src/Tests/CyclicTestsRegulatory.cpp \
    Src/Tests/CyclicTestsShutoff.cpp \
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
    ObjectWindow.h \
    OtherTestSettings.h \
    Program.h \
    Registry.h \
    SelectTests.h \
    Sensor.h \
    Src/CustomChart/MyChart.h \
    Src/CustomChart/MySeries.h \
    Src/LabeledSlider/LabeledSlider.h \
    Src/Mpi/Mpi.h \
    Src/Mpi/MpiSettings.h \
    Src/ReportBuilders/ReportBuilder.h \
    Src/ReportBuilders/ReportBuilder_B_CVT.h \
    Src/ReportBuilders/ReportBuilder_B_SACVT.h \
    Src/ReportBuilders/ReportBuilder_C_CVT.h \
    Src/ReportBuilders/ReportBuilder_C_SACVT.h \
    Src/ReportBuilders/ReportBuilder_C_SOVT.h \
    Src/ReportBuilders/ReportSaver.h \
    Src/Runners/AbstractTestRunner.h \
    Src/Runners/BaseRunner.h \
    Src/Runners/CyclicRegulatoryRunner.h \
    Src/Runners/CyclicShutoffRunner.h \
    Src/Runners/MainTestRunner.h \
    Src/Runners/OptionResolutionRunner.h \
    Src/Runners/OptionResponseRunner.h \
    Src/Runners/RunnerFactory.h \
    Src/Runners/StepTestRunner.h \
    Src/Runners/StrokeTestRunner.h \
    Src/Telemetry/TelemetryStore.h \
    Src/Tests/CyclicTestPositioner.h \
    Src/Tests/CyclicTests.h \
    Src/Tests/CyclicTestsRegulatory.h \
    Src/Tests/CyclicTestsShutoff.h \
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
    img.qrc \
    translations.qrc
