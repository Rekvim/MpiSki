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

INCLUDEPATH += $$PWD/Src/CustomChart

SOURCES += \
    Src/CustomChart/ChartImageService.cpp \
    Src/CustomChart/ChartManager.cpp \
    Src/Domain/DeviceInitializer.cpp \
    Src/Domain/Mpi/Device.cpp \
    Src/Domain/Mpi/Settings.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAlgorithm.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAnalyzer.cpp \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAnalyzer.cpp \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffRunner.cpp \
    Src/Domain/Tests/Main/MainAlgorithm.cpp \
    Src/Domain/Tests/Main/MainAnalyzer.cpp \
    Src/Domain/Tests/Main/MainRunner.cpp \
    Src/Domain/Tests/Option/OptionAlgorithm.cpp \
    Src/Domain/Tests/Option/Step/StepAlgorithm.cpp \
    Src/Domain/Tests/Option/Step/StepAnalyzer.cpp \
    Src/Domain/Tests/Option/Step/StepRunner.cpp \
    Src/Domain/Tests/Stroke/StrokeAlgorithm.cpp \
    Src/Domain/Tests/Stroke/StrokeAnalyzer.cpp \
    Src/Domain/Tests/Stroke/StrokeRunner.cpp \
    Src/Domain/Uart/Message.cpp \
    Src/Domain/Uart/Port.cpp \
    Src/Domain/Uart/Reader.cpp \
    Src/Report/Patterns/B_CVT.cpp \
    Src/Report/Patterns/B_SACVT.cpp \
    Src/Report/Patterns/C_CVT.cpp \
    Src/Report/Patterns/C_SACVT.cpp \
    Src/Report/Patterns/C_SOVT.cpp \
    Src/Report/Blocks/TechnicalResults.cpp \
    Src/Report/Saver.cpp \
    Src/Storage/AppSettings.cpp \
    Src/Gui/MainWindow/CrossingIndicatorsPresenter.cpp \
    Src/Gui/MainWindow/TelemetryUiMapper.cpp \
    Src/Gui/MainWindow/TestController.cpp \
    Src/Gui/Setup/ValveWindow/Logic.cpp \
    Src/Gui/Setup/ValveWindow/Mapper.cpp \
    Src/Utils/NumberUtils.cpp \
    Src/Gui/Setup/ObjectWindow.cpp \
    Src/Gui/Setup/ValveWindow/ValveWindow.cpp \
    Src/Gui/Setup/SelectTests.cpp \
    Src/Gui/MainWindow/MainWindow.cpp \
    Src/Gui/TestSettings/CyclicTestSettings.cpp \
    Src/Gui/TestSettings/OtherTestSettings.cpp \
    Src/Gui/TestSettings/StepTestSettings.cpp \
    Src/Gui/TestSettings/MainTestSettings.cpp \
    Src/CustomChart/MyChart.cpp \
    Src/CustomChart/MySeries.cpp \
    Src/LabeledSlider/LabeledSlider.cpp \
    Src/Domain/Tests/BaseRunner.cpp \
    Src/Domain/Tests/Test.cpp \
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp \
    Src/Domain/Tests/Option/Response/ResponseRunner.cpp \
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.cpp \
    Src/ValidatorFactory/ValidatorFactory.cpp \
    Src/Domain/Program.cpp \
    Src/Storage/Registry.cpp \
    Src/Domain/Measurement/Sensor.cpp \
    main.cpp \

HEADERS += \
    Src/CustomChart/ChartImageService.h \
    Src/CustomChart/ChartManager.h \
    Src/Domain/DeviceInitializer.h \
    Src/Domain/Measurement/Sample.h \
    Src/Domain/Measurement/Sensor.h \
    Src/Domain/Measurement/TestDataBuffer.h \
    Src/Domain/Mpi/Device.h \
    Src/Domain/Mpi/Settings.h \
    Src/Domain/Tests/AnalyzerFactory.h \
    Src/Domain/Tests/Cyclic/CyclicParams.h \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAlgorithm.h \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAnalyzer.h \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryParams.h \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryResult.h \
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.h \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.h \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAnalyzer.h \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffParams.h \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffResult.h \
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffRunner.h \
    Src/Domain/Tests/IAnalyzer.h \
    Src/Domain/Tests/Main/MainAlgorithm.h \
    Src/Domain/Tests/Main/MainAnalyzer.h \
    Src/Domain/Tests/Main/MainParams.h \
    Src/Domain/Tests/Main/MainResult.h \
    Src/Domain/Tests/Main/MainRunner.h \
    Src/Domain/Tests/Option/OptionAlgorithm.h \
    Src/Domain/Tests/Option/Step/StepAlgorithm.h \
    Src/Domain/Tests/Option/Step/StepAnalyzer.h \
    Src/Domain/Tests/Option/Step/StepParams.h \
    Src/Domain/Tests/Option/Step/StepResult.h \
    Src/Domain/Tests/Option/Step/StepRunner.h \
    Src/Domain/Tests/Stroke/StrokeAlgorithm.h \
    Src/Domain/Tests/Stroke/StrokeAnalyzer.h \
    Src/Domain/Tests/Stroke/StrokeResult.h \
    Src/Domain/Tests/Stroke/StrokeRunner.h \
    Src/Domain/Uart/Message.h \
    Src/Domain/Uart/Port.h \
    Src/Domain/Uart/Reader.h \
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
    Src/Gui/MainWindow/CrossingIndicatorsPresenter.h \
    Src/Gui/MainWindow/TelemetryUiMapper.h \
    Src/Gui/MainWindow/TestController.h \
    Src/Gui/Setup/ObjectWindow.h \
    Src/Gui/Setup/ValveWindow/Logic.h \
    Src/Gui/Setup/ValveWindow/ValveEnums.h \
    Src/Gui/Setup/ValveWindow/ValveWindow.h \
    Src/Gui/Setup/SelectTests.h \
    Src/Gui/Setup/ValveWindow/Mapper.h \
    Src/Gui/TestSettings/AbstractTestSettings.h \
    Src/Gui/TestSettings/CyclicTestSettings.h \
    Src/Gui/TestSettings/OtherTestSettings.h \
    Src/Gui/TestSettings/StepTestSettings.h \
    Src/Gui/TestSettings/MainTestSettings.h \
    Src/Gui/MainWindow/MainWindow.h \
    Src/Domain/Program.h \
    Src/Storage/Registry.h \
    Src/CustomChart/MyChart.h \
    Src/CustomChart/MySeries.h \
    Src/LabeledSlider/LabeledSlider.h \
    Src/Domain/Tests/Test.h \
    Src/Domain/Tests/BaseRunner.h \
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.h \
    Src/Domain/Tests/Option/OptionTestParams.h \
    Src/Domain/Tests/Option/Response/ResponseRunner.h \
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.h \
    Src/Utils/NumberUtils.h \
    Src/Utils/Shortcuts/TabActionRouter.h \
    Src/Utils/Shortcuts/TabBinder.h \
    Src/Utils/SignalUtils.h \
    Src/ValidatorFactory/RegexPatterns.h \
    Src/ValidatorFactory/ValidatorFactory.h

FORMS += \
    Src/Gui/Setup/ObjectWindow.ui \
    Src/Gui/Setup/SelectTests.ui \
    Src/Gui/Setup/ValveWindow/ValveWindow.ui \
    Src/Gui/MainWindow/MainWindow.ui \
    Src/Gui/TestSettings/CyclicTestSettings.ui \
    Src/Gui/TestSettings/MainTestSettings.ui \
    Src/Gui/TestSettings/OtherTestSettings.ui \
    Src/Gui/TestSettings/StepTestSettings.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    excel.qrc \
    img.qrc \
    translations.qrc