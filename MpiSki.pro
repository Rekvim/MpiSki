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
    Src/Domain/Tests/BaseRunner.cpp \
    Src/Domain/Tests/Test.cpp \
    Src/Domain/Tests/Stroke/StrokeTestRunner.cpp \
    Src/Domain/Tests/Stroke/StrokeTest.cpp \
    Src/Domain/Tests/Stroke/StrokeTestAnalyzer.cpp \
    Src/Domain/Tests/Main/MainTestRunner.cpp \
    Src/Domain/Tests/Main/MainTest.cpp \
    Src/Domain/Tests/Main/MainTestAnalyzer.cpp \
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp \
    Src/Domain/Tests/Option/OptionTest.cpp \
    Src/Domain/Tests/Option/Response/ResponseRunner.cpp \
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.cpp \
    Src/Domain/Tests/Option/Step/StepTestRunner.cpp \
    Src/Domain/Tests/Option/Step/StepTest.cpp \
    Src/Domain/Tests/Option/Step/StepTestAnalyzer.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryRunner.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicTestsRegulatory.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicShutoffAnalyzer.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicShutoffRunner.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicTestsShutoff.cpp \
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
    Src/Domain/Tests/AnalyzerFactory.h \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryParams.h \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicShutoffAnalyzer.h \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicShutoffParams.h \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicShutoffResult.h \
    Src/Domain/Tests/IAnalyzer.h \
    Src/Domain/Tests/ITestRunner.h \
    Src/ReportBuilders/ReportBlocks/CyclicRangesBlock.h \
    Src/ReportBuilders/ReportBlocks/CyclicSummaryBlock.h \
    Src/ReportBuilders/ReportBlocks/IReportBlock.h \
    Src/ReportBuilders/ReportBlocks/ObjectInfoBlock.h \
    Src/ReportBuilders/ReportBlocks/SolenoidDetailsBlock.h \
    Src/ReportBuilders/ReportBlocks/StepReactionBlock.h \
    Src/ReportBuilders/ReportBlocks/TechnicalResultsBlock.h \
    Src/ReportBuilders/ReportBlocks/ValveSpecBlock.h \
    Src/ReportBuilders/ReportBuilderFactory.h \
    Src/ReportBuilders/ReportWriter.h \
    Src/Storage/AppSettings.h \
    Src/Storage/SettingsGroup.h \
    Src/Storage/Telemetry.h \
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
    Src/Domain/Tests/Test.h \
    Src/Domain/Tests/Main/MainTestAnalyzer.h \
    Src/Domain/Tests/AbstractTestRunner.h \
    Src/Domain/Tests/BaseRunner.h \
    Src/Domain/Tests/Stroke/StrokeTestRunner.h \
    Src/Domain/Tests/Stroke/StrokeTest.h \
    Src/Domain/Tests/Stroke/StrokeTestAnalyzer.h \
    Src/Domain/Tests/Stroke/StrokeTestResult.h \
    Src/Domain/Tests/CyclicTestPositioner.h \
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.h \
    Src/Domain/Tests/Cyclic/CyclicTestParams.h \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryRunner.h \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicTestsRegulatory.h \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.h \
    Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryTestResult.h \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicShutoffRunner.h \
    Src/Domain/Tests/Cyclic/Shutoff/CyclicTestsShutoff.h \
    Src/Domain/Tests/Main/MainTestParams.h \
    Src/Domain/Tests/Main/MainTestRunner.h \
    Src/Domain/Tests/Main/MainTest.h \
    Src/Domain/Tests/Main/MainTestResult.h \
    Src/Domain/Tests/Option/OptionTest.h \
    Src/Domain/Tests/Option/OptionTestParams.h \
    Src/Domain/Tests/Option/Response/ResponseRunner.h \
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.h \
    Src/Domain/Tests/Option/Step/StepTestParams.h \
    Src/Domain/Tests/Option/Step/StepTest.h \
    Src/Domain/Tests/Option/Step/StepTestRunner.h \
    Src/Domain/Tests/Option/Step/StepTestAnalyzer.h \
    Src/Domain/Tests/Option/Step//StepTestResult.h \
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
