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
    Src/Domain/Tests/Cyclic/Regulatory/Algorithm.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/Analyzer.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/Runner.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/Algorithm.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/Analyzer.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/Runner.cpp \
    Src/Domain/Tests/Main/Algorithm.cpp \
    Src/Domain/Tests/Main/Analyzer.cpp \
    Src/Domain/Tests/Main/Runner.cpp \
    Src/Report/Patterns/B_CVT.cpp \
    Src/Report/Patterns/B_SACVT.cpp \
    Src/Report/Patterns/C_CVT.cpp \
    Src/Report/Patterns/C_SACVT.cpp \
    Src/Report/Patterns/C_SOVT.cpp \
    Src/Report/Blocks/TechnicalResults.cpp \
    Src/Report/Saver.cpp \
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
    Src/Domain/Tests/BaseRunner.cpp \
    Src/Domain/Tests/Test.cpp \
    Src/Domain/Tests/Stroke/StrokeTestRunner.cpp \
    Src/Domain/Tests/Stroke/StrokeTest.cpp \
    Src/Domain/Tests/Stroke/StrokeTestAnalyzer.cpp \
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp \
    Src/Domain/Tests/Option/OptionTest.cpp \
    Src/Domain/Tests/Option/Response/ResponseRunner.cpp \
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.cpp \
    Src/Domain/Tests/Option/Step/StepTestRunner.cpp \
    Src/Domain/Tests/Option/Step/StepTest.cpp \
    Src/Domain/Tests/Option/Step/StepTestAnalyzer.cpp \
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
    Src/Domain/Tests/Cyclic/Params.h \
    Src/Domain/Tests/Cyclic/Regulatory/Algorithm.h \
    Src/Domain/Tests/Cyclic/Regulatory/Analyzer.h \
    Src/Domain/Tests/Cyclic/Regulatory/Params.h \
    Src/Domain/Tests/Cyclic/Regulatory/Result.h \
    Src/Domain/Tests/Cyclic/Regulatory/Runner.h \
    Src/Domain/Tests/Cyclic/Shutoff/Algorithm.h \
    Src/Domain/Tests/Cyclic/Shutoff/Analyzer.h \
    Src/Domain/Tests/Cyclic/Shutoff/Params.h \
    Src/Domain/Tests/Cyclic/Shutoff/Result.h \
    Src/Domain/Tests/Cyclic/Shutoff/Runner.h \
    Src/Domain/Tests/IAnalyzer.h \
    Src/Domain/Tests/Main/Algorithm.h \
    Src/Domain/Tests/Main/Analyzer.h \
    Src/Domain/Tests/Main/Params.h \
    Src/Domain/Tests/Main/Result.h \
    Src/Domain/Tests/Main/Runner.h \
    Src/Report/Builder.h \
    Src/Report/BuilderFactory.h \
    Src/Report/Patterns/B_CVT.h \
    Src/Report/Patterns/B_SACVT.h \
    Src/Report/Patterns/C_CVT.h \
    Src/Report/Patterns/C_SACVT.h \
    Src/Report/Patterns/C_SOVT.h \
    Src/Report/Blocks/CyclicRanges.h \
    Src/Report/Blocks/CyclicSummary.h \
    Src/Report/Blocks/IBlock.h \
    Src/Report/Blocks/ObjectInfo.h \
    Src/Report/Blocks/SolenoidDetails.h \
    Src/Report/Blocks/StepReaction.h \
    Src/Report/Blocks/TechnicalResults.h \
    Src/Report/Blocks/ValveSpec.h \
    Src/Report/Saver.h \
    Src/Report/Writer.h \
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
    Src/Domain/Tests/Test.h \
    Src/Domain/Tests/AbstractTestRunner.h \
    Src/Domain/Tests/BaseRunner.h \
    Src/Domain/Tests/Stroke/StrokeTestRunner.h \
    Src/Domain/Tests/Stroke/StrokeTest.h \
    Src/Domain/Tests/Stroke/StrokeTestAnalyzer.h \
    Src/Domain/Tests/Stroke/StrokeTestResult.h \
    Src/Domain/Tests/CyclicTestPositioner.h \
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.h \
    Src/Domain/Tests/Option/OptionTest.h \
    Src/Domain/Tests/Option/OptionTestParams.h \
    Src/Domain/Tests/Option/Response/ResponseRunner.h \
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.h \
    Src/Domain/Tests/Option/Step/StepTestParams.h \
    Src/Domain/Tests/Option/Step/StepTest.h \
    Src/Domain/Tests/Option/Step/StepTestRunner.h \
    Src/Domain/Tests/Option/Step/StepTestAnalyzer.h \
    Src/Domain/Tests/Option/Step/StepTestResult.h \
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