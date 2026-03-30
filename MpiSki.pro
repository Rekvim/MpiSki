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
    Src/CustomChart/ChartImageService.cpp \
    Src/CustomChart/ChartManager.cpp \
    Src/Domain/DeviceInitializer.cpp \
    Src/ReportBuilders/ReportBlocks/TechnicalResultsBlock.cpp \
    Src/Storage/AppSettings.cpp \
    Src/Storage/Telemetry.cpp \
    Src/Tests/Analyzer/StepTestAnalyzer.cpp \
    Src/Tests/Analyzer/StrokeTestAnalyzer.cpp \
    Src/Ui/MainWindow/CrossingIndicatorsPresenter.cpp \
    Src/Ui/MainWindow/TelemetryUiMapper.cpp \
    Src/Ui/MainWindow/TestController.cpp \
    Src/Ui/Setup/ValveWindow/Logic.cpp \
    Src/Ui/Setup/ValveWindow/Mapper.cpp \
    Src/Utils/NumberUtils.cpp \
    main.cpp \
    Src/Ui/Setup/ObjectWindow.cpp \
    Src/Ui/Setup/ValveWindow/ValveWindow.cpp \
    Src/Ui/Setup/SelectTests.cpp \
    Src/Ui/MainWindow/MainWindow.cpp \
    Src/Ui/TestSettings/CyclicTestSettings.cpp \
    Src/Ui/TestSettings/OtherTestSettings.cpp \
    Src/Ui/TestSettings/StepTestSettings.cpp \
    Src/Ui/TestSettings/MainTestSettings.cpp \
    Src/CustomChart/MyChart.cpp \
    Src/CustomChart/MySeries.cpp \
    Src/LabeledSlider/LabeledSlider.cpp \
    Src/Domain/Mpi/Mpi.cpp \
    Src/Domain/Mpi/MpiSettings.cpp \
    Src/ReportBuilders/Patterns/ReportBuilder_B_CVT.cpp \
    Src/ReportBuilders/Patterns/ReportBuilder_B_SACVT.cpp \
    Src/ReportBuilders/Patterns/ReportBuilder_C_CVT.cpp \
    Src/ReportBuilders/Patterns/ReportBuilder_C_SACVT.cpp \
    Src/ReportBuilders/Patterns/ReportBuilder_C_SOVT.cpp \
    Src/ReportBuilders/ReportSaver.cpp \
    Src/Runners/BaseRunner.cpp \
    Src/Runners/CyclicRegulatoryRunner.cpp \
    Src/Runners/CyclicShutoffRunner.cpp \
    Src/Runners/MainTestRunner.cpp \
    Src/Runners/OptionResolutionRunner.cpp \
    Src/Runners/OptionResponseRunner.cpp \
    Src/Runners/StepTestRunner.cpp \
    Src/Runners/StrokeTestRunner.cpp \
    Src/Tests/CyclicTestPositioner.cpp \
    Src/Tests/CyclicTests.cpp \
    Src/Tests/CyclicTestsRegulatory.cpp \
    Src/Tests/CyclicTestsShutoff.cpp \
    Src/Tests/MainTest.cpp \
    Src/Tests/OptionTest.cpp \
    Src/Tests/StepTest.cpp \
    Src/Tests/StrokeTest.cpp \
    Src/Tests/Test.cpp \
    Src/Domain/Uart/Uart.cpp \
    Src/Domain/Uart/UartMessage.cpp \
    Src/Domain/Uart/UartReader.cpp \
    Src/ValidatorFactory/ValidatorFactory.cpp \
    Src/Domain/Program.cpp \
    Src/Storage/Registry.cpp \
    Src/Domain/Sensor.cpp

HEADERS += \
    Src/CustomChart/ChartImageService.h \
    Src/CustomChart/ChartManager.h \
    Src/Domain/DeviceInitializer.h \
    Src/Domain/Measurement/Sample.h \
    Src/Domain/Measurement/TestDataBuffer.h \
    Src/ReportBuilders/ReportBlocks/CyclicRangesBlock.h \
    Src/ReportBuilders/ReportBlocks/CyclicSummaryBlock.h \
    Src/ReportBuilders/ReportBlocks/IReportBlock.h \
    Src/ReportBuilders/ReportBlocks/ObjectInfoBlock.h \
    Src/ReportBuilders/ReportBlocks/SolenoidDetailsBlock.h \
    Src/ReportBuilders/ReportBlocks/StepReactionBlock.h \
    Src/ReportBuilders/ReportBlocks/TechnicalResultsBlock.h \
    Src/ReportBuilders/ReportBlocks/ValveSpecBlock.h \
    Src/ReportBuilders/ReportWriter.h \
    Src/Storage/AppSettings.h \
    Src/Storage/SettingsGroup.h \
    Src/Storage/Telemetry.h \
    Src/Tests/Analyzer/StepTestAnalyzer.h \
    Src/Tests/Analyzer/StrokeTestAnalyzer.h \
    Src/Ui/MainWindow/CrossingIndicatorsPresenter.h \
    Src/Ui/MainWindow/TelemetryUiMapper.h \
    Src/Ui/MainWindow/TestController.h \
    Src/Ui/Setup/ObjectWindow.h \
    Src/Ui/Setup/ValveWindow/Logic.h \
    Src/Ui/Setup/ValveWindow/ValveEnums.h \
    Src/Ui/Setup/ValveWindow/ValveWindow.h \
    Src/Ui/Setup/SelectTests.h \
    Src/Ui/Setup/ValveWindow/Mapper.h \
    Src/Ui/TestSettings/AbstractTestSettings.h \
    Src/Ui/TestSettings/CyclicTestSettings.h \
    Src/Ui/TestSettings/ITestSettings.h \
    Src/Ui/TestSettings/OtherTestSettings.h \
    Src/Ui/TestSettings/StepTestSettings.h \
    Src/Ui/TestSettings/MainTestSettings.h \
    Src/Ui/MainWindow/MainWindow.h \
    Src/Domain/Program.h \
    Src/Storage/Registry.h \
    Src/Domain/Sensor.h \
    Src/CustomChart/MyChart.h \
    Src/CustomChart/MySeries.h \
    Src/LabeledSlider/LabeledSlider.h \
    Src/Domain/Mpi/Mpi.h \
    Src/Domain/Mpi/MpiSettings.h \
    Src/ReportBuilders/ReportBuilder.h \
    Src/ReportBuilders/Patterns/ReportBuilder_B_CVT.h \
    Src/ReportBuilders/Patterns/ReportBuilder_B_SACVT.h \
    Src/ReportBuilders/Patterns/ReportBuilder_C_CVT.h \
    Src/ReportBuilders/Patterns/ReportBuilder_C_SACVT.h \
    Src/ReportBuilders/Patterns/ReportBuilder_C_SOVT.h \
    Src/ReportBuilders/ReportSaver.h \
    Src/Runners/AbstractTestRunner.h \
    Src/Runners/BaseRunner.h \
    Src/Runners/CyclicRegulatoryRunner.h \
    Src/Runners/CyclicShutoffRunner.h \
    Src/Runners/MainTestRunner.h \
    Src/Runners/OptionResolutionRunner.h \
    Src/Runners/OptionResponseRunner.h \
    Src/Runners/StepTestRunner.h \
    Src/Runners/StrokeTestRunner.h \
    Src/Tests/CyclicTestPositioner.h \
    Src/Tests/CyclicTests.h \
    Src/Tests/CyclicTestsRegulatory.h \
    Src/Tests/CyclicTestsShutoff.h \
    Src/Tests/MainTest.h \
    Src/Tests/OptionTest.h \
    Src/Tests/StepTest.h \
    Src/Tests/StrokeTest.h \
    Src/Tests/Test.h \
    Src/Domain/Uart/Uart.h \
    Src/Domain/Uart/UartMessage.h \
    Src/Domain/Uart/UartReader.h \
    Src/Utils/NumberUtils.h \
    Src/Utils/Shortcuts/TabActionRouter.h \
    Src/Utils/Shortcuts/TabBinder.h \
    Src/Utils/SignalUtils.h \
    Src/ValidatorFactory/RegexPatterns.h \
    Src/ValidatorFactory/ValidatorFactory.h

FORMS += \
    Src/Ui/Setup/ObjectWindow.ui \
    Src/Ui/Setup/SelectTests.ui \
    Src/Ui/Setup/ValveWindow/ValveWindow.ui \
    Src/Ui/MainWindow/MainWindow.ui \
    Src/Ui/TestSettings/CyclicTestSettings.ui \
    Src/Ui/TestSettings/MainTestSettings.ui \
    Src/Ui/TestSettings/OtherTestSettings.ui \
    Src/Ui/TestSettings/StepTestSettings.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    excel.qrc \
    img.qrc \
    translations.qrc
