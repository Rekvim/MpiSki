QT += core gui widgets
QT += serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += object_parallel_to_source

# MOC_DIR = build/moc
# RCC_DIR = build/rcc
# UI_DIR = build/ui
# OBJECTS_DIR = build/obj

# static release

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QXLSX_PARENTPATH = $$PWD/QXlsx/
QXLSX_HEADERPATH = $$PWD/QXlsx/header/
QXLSX_SOURCEPATH = $$PWD/QXlsx/source/

include($$PWD/QXlsx/QXlsx.pri)

INCLUDEPATH += $$PWDCustomChart

SOURCES += \
    CustomChart/ChartImageService.cpp \
    CustomChart/ChartManager.cpp \
    Domain/DeviceInitializer.cpp \
    Domain/Mpi/Device.cpp \
    Domain/Mpi/Settings.cpp \
    Domain/Tests/Cyclic/Regulatory/RegulatoryAlgorithm.cpp \
    Domain/Tests/Cyclic/Regulatory/RegulatoryAnalyzer.cpp \
    Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.cpp \
    Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.cpp \
    Domain/Tests/Cyclic/Shutoff/ShutoffAnalyzer.cpp \
    Domain/Tests/Cyclic/Shutoff/ShutoffRunner.cpp \
    Domain/Tests/Main/MainAlgorithm.cpp \
    Domain/Tests/Main/MainAnalyzer.cpp \
    Domain/Tests/Main/MainRunner.cpp \
    Domain/Tests/Option/OptionAlgorithm.cpp \
    Domain/Tests/Option/Step/StepAlgorithm.cpp \
    Domain/Tests/Option/Step/StepAnalyzer.cpp \
    Domain/Tests/Option/Step/StepRunner.cpp \
    Domain/Tests/Stroke/StrokeAlgorithm.cpp \
    Domain/Tests/Stroke/StrokeAnalyzer.cpp \
    Domain/Tests/Stroke/StrokeRunner.cpp \
    Domain/Uart/Message.cpp \
    Domain/Uart/Port.cpp \
    Domain/Uart/Reader.cpp \
    Report/Patterns/B_CVT.cpp \
    Report/Patterns/B_SACVT.cpp \
    Report/Patterns/C_CVT.cpp \
    Report/Patterns/C_SACVT.cpp \
    Report/Patterns/C_SOVT.cpp \
    Report/Blocks/TechnicalResults.cpp \
    Report/Saver.cpp \
    Storage/AppSettings.cpp \
    Gui/MainWindow/CrossingIndicatorsPresenter.cpp \
    Gui/MainWindow/TelemetryUiMapper.cpp \
    Gui/MainWindow/TestController.cpp \
    Gui/Setup/ValveWindow/Logic.cpp \
    Gui/Setup/ValveWindow/Mapper.cpp \
    Utils/NumberUtils.cpp \
    Gui/Setup/ObjectWindow.cpp \
    Gui/Setup/ValveWindow/ValveWindow.cpp \
    Gui/Setup/SelectTests.cpp \
    Gui/MainWindow/MainWindow.cpp \
    Gui/TestSettings/CyclicTestSettings.cpp \
    Gui/TestSettings/OtherTestSettings.cpp \
    Gui/TestSettings/StepTestSettings.cpp \
    Gui/TestSettings/MainTestSettings.cpp \
    CustomChart/MyChart.cpp \
    CustomChart/MySeries.cpp \
    LabeledSlider/LabeledSlider.cpp \
    Domain/Tests/BaseRunner.cpp \
    Domain/Tests/Test.cpp \
    Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp \
    Domain/Tests/Option/Response/ResponseRunner.cpp \
    Domain/Tests/Option/Resolution/ResolutionRunner.cpp \
    ValidatorFactory/ValidatorFactory.cpp \
    Domain/Program.cpp \
    Storage/Registry.cpp \
    Domain/Measurement/Sensor.cpp \
    main.cpp \

HEADERS += \
    CustomChart/ChartImageService.h \
    CustomChart/ChartManager.h \
    Domain/DeviceInitializer.h \
    Domain/Measurement/Sample.h \
    Domain/Measurement/Sensor.h \
    Domain/Measurement/TestDataBuffer.h \
    Domain/Mpi/Device.h \
    Domain/Mpi/Settings.h \
    Domain/Tests/AnalyzerFactory.h \
    Domain/Tests/Cyclic/CyclicParams.h \
    Domain/Tests/Cyclic/Regulatory/RegulatoryAlgorithm.h \
    Domain/Tests/Cyclic/Regulatory/RegulatoryAnalyzer.h \
    Domain/Tests/Cyclic/Regulatory/RegulatoryParams.h \
    Domain/Tests/Cyclic/Regulatory/RegulatoryResult.h \
    Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.h \
    Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.h \
    Domain/Tests/Cyclic/Shutoff/ShutoffAnalyzer.h \
    Domain/Tests/Cyclic/Shutoff/ShutoffParams.h \
    Domain/Tests/Cyclic/Shutoff/ShutoffResult.h \
    Domain/Tests/Cyclic/Shutoff/ShutoffRunner.h \
    Domain/Tests/IAnalyzer.h \
    Domain/Tests/Main/MainAlgorithm.h \
    Domain/Tests/Main/MainAnalyzer.h \
    Domain/Tests/Main/MainParams.h \
    Domain/Tests/Main/MainResult.h \
    Domain/Tests/Main/MainRunner.h \
    Domain/Tests/Option/OptionAlgorithm.h \
    Domain/Tests/Option/Step/StepAlgorithm.h \
    Domain/Tests/Option/Step/StepAnalyzer.h \
    Domain/Tests/Option/Step/StepParams.h \
    Domain/Tests/Option/Step/StepResult.h \
    Domain/Tests/Option/Step/StepRunner.h \
    Domain/Tests/Stroke/StrokeAlgorithm.h \
    Domain/Tests/Stroke/StrokeAnalyzer.h \
    Domain/Tests/Stroke/StrokeResult.h \
    Domain/Tests/Stroke/StrokeRunner.h \
    Domain/Uart/Message.h \
    Domain/Uart/Port.h \
    Domain/Uart/Reader.h \
    Report/Builder.h \
    Report/BuilderFactory.h \
    Report/Patterns/B_CVT.h \
    Report/Patterns/B_SACVT.h \
    Report/Patterns/C_CVT.h \
    Report/Patterns/C_SACVT.h \
    Report/Patterns/C_SOVT.h \
    Report/Blocks/CyclicRanges.h \
    Report/Blocks/CyclicSummary.h \
    Report/Blocks/IBlock.h \
    Report/Blocks/ObjectInfo.h \
    Report/Blocks/SolenoidDetails.h \
    Report/Blocks/StepReaction.h \
    Report/Blocks/TechnicalResults.h \
    Report/Blocks/ValveSpec.h \
    Report/Saver.h \
    Report/Writer.h \
    Storage/AppSettings.h \
    Storage/SettingsGroup.h \
    Storage/Telemetry.h \
    Gui/MainWindow/CrossingIndicatorsPresenter.h \
    Gui/MainWindow/TelemetryUiMapper.h \
    Gui/MainWindow/TestController.h \
    Gui/Setup/ObjectWindow.h \
    Gui/Setup/ValveWindow/Logic.h \
    Gui/Setup/ValveWindow/ValveEnums.h \
    Gui/Setup/ValveWindow/ValveWindow.h \
    Gui/Setup/SelectTests.h \
    Gui/Setup/ValveWindow/Mapper.h \
    Gui/TestSettings/AbstractTestSettings.h \
    Gui/TestSettings/CyclicTestSettings.h \
    Gui/TestSettings/OtherTestSettings.h \
    Gui/TestSettings/StepTestSettings.h \
    Gui/TestSettings/MainTestSettings.h \
    Gui/MainWindow/MainWindow.h \
    Domain/Program.h \
    Storage/Registry.h \
    CustomChart/MyChart.h \
    CustomChart/MySeries.h \
    LabeledSlider/LabeledSlider.h \
    Domain/Tests/Test.h \
    Domain/Tests/BaseRunner.h \
    Domain/Tests/CyclicPositioner/CyclicTestPositioner.h \
    Domain/Tests/Option/OptionTestParams.h \
    Domain/Tests/Option/Response/ResponseRunner.h \
    Domain/Tests/Option/Resolution/ResolutionRunner.h \
    Utils/NumberUtils.h \
    Utils/Shortcuts/TabActionRouter.h \
    Utils/Shortcuts/TabBinder.h \
    Utils/SignalUtils.h \
    ValidatorFactory/RegexPatterns.h \
    ValidatorFactory/ValidatorFactory.h

FORMS += \
    Gui/Setup/ObjectWindow.ui \
    Gui/Setup/SelectTests.ui \
    Gui/Setup/ValveWindow/ValveWindow.ui \
    Gui/MainWindow/MainWindow.ui \
    Gui/TestSettings/CyclicTestSettings.ui \
    Gui/TestSettings/MainTestSettings.ui \
    Gui/TestSettings/OtherTestSettings.ui \
    Gui/TestSettings/StepTestSettings.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    excel.qrc \
    img.qrc \
    translations.qrc