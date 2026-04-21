cmake_minimum_required(VERSION 3.21)

project(MpiSki VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    SerialPort
    Charts
)

qt_standard_project_setup()

# ------------------------------------------------------------
# QXlsx
# ------------------------------------------------------------
# Для сторонней библиотеки на этапе миграции допустимо использовать GLOB.
# Позже лучше заменить на явный список файлов или отдельный CMakeLists.txt внутри QXlsx.

file(GLOB QXLSX_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/QXlsx/header/*.h"
)

file(GLOB QXLSX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/QXlsx/source/*.cpp"
)

add_library(QXlsx STATIC
    ${QXLSX_HEADERS}
    ${QXLSX_SOURCES}
)

target_include_directories(QXlsx
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/QXlsx/header"
        "${CMAKE_CURRENT_SOURCE_DIR}/QXlsx/source"
)

target_link_libraries(QXlsx
    PUBLIC
        Qt6::Core
        Qt6::Gui
)

# Иногда QXlsx требует это определение, если собирается как shared.
# У тебя раньше оно было закомментировано, поэтому пока не включаю.
# target_compile_definitions(QXlsx PRIVATE QXLSX_SHAREDLIB QXLSX_EXPORTS)

# ------------------------------------------------------------
# Main application
# ------------------------------------------------------------
qt_add_executable(${PROJECT_NAME}
    main.cpp

    # Sources
    Src/CustomChart/ChartImageService.cpp
    Src/CustomChart/ChartManager.cpp
    Src/Domain/DeviceInitializer.cpp
    Src/Domain/Mpi/Device.cpp
    Src/Domain/Mpi/Settings.cpp
    Src/Domain/Tests/Cyclic/Regulatory/Algorithm.cpp
    Src/Domain/Tests/Cyclic/Regulatory/Analyzer.cpp
    Src/Domain/Tests/Cyclic/Regulatory/Runner.cpp
    Src/Domain/Tests/Cyclic/Shutoff/Algorithm.cpp
    Src/Domain/Tests/Cyclic/Shutoff/Analyzer.cpp
    Src/Domain/Tests/Cyclic/Shutoff/Runner.cpp
    Src/Domain/Tests/Main/Algorithm.cpp
    Src/Domain/Tests/Main/Analyzer.cpp
    Src/Domain/Tests/Main/Runner.cpp
    Src/Domain/Tests/Option/Algorithm.cpp
    Src/Domain/Tests/Option/Step/Algorithm.cpp
    Src/Domain/Tests/Option/Step/Analyzer.cpp
    Src/Domain/Tests/Option/Step/Runner.cpp
    Src/Domain/Tests/Stroke/Algorithm.cpp
    Src/Domain/Tests/Stroke/Analyzer.cpp
    Src/Domain/Tests/Stroke/Runner.cpp
    Src/Domain/Uart/Message.cpp
    Src/Domain/Uart/Port.cpp
    Src/Domain/Uart/Reader.cpp
    Src/Report/Patterns/B_CVT.cpp
    Src/Report/Patterns/B_SACVT.cpp
    Src/Report/Patterns/C_CVT.cpp
    Src/Report/Patterns/C_SACVT.cpp
    Src/Report/Patterns/C_SOVT.cpp
    Src/Report/Blocks/TechnicalResults.cpp
    Src/Report/Saver.cpp
    Src/Storage/AppSettings.cpp
    Src/Gui/MainWindow/CrossingIndicatorsPresenter.cpp
    Src/Gui/MainWindow/TelemetryUiMapper.cpp
    Src/Gui/MainWindow/TestController.cpp
    Src/Gui/Setup/ValveWindow/Logic.cpp
    Src/Gui/Setup/ValveWindow/Mapper.cpp
    Src/Utils/NumberUtils.cpp
    Src/Gui/Setup/ObjectWindow.cpp
    Src/Gui/Setup/ValveWindow/ValveWindow.cpp
    Src/Gui/Setup/SelectTests.cpp
    Src/Gui/MainWindow/MainWindow.cpp
    Src/Gui/TestSettings/CyclicTestSettings.cpp
    Src/Gui/TestSettings/OtherTestSettings.cpp
    Src/Gui/TestSettings/StepTestSettings.cpp
    Src/Gui/TestSettings/MainTestSettings.cpp
    Src/CustomChart/MyChart.cpp
    Src/CustomChart/MySeries.cpp
    Src/LabeledSlider/LabeledSlider.cpp
    Src/Domain/Tests/BaseRunner.cpp
    Src/Domain/Tests/Test.cpp
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp
    Src/Domain/Tests/Option/Response/Runner.cpp
    Src/Domain/Tests/Option/Resolution/Runner.cpp
    Src/ValidatorFactory/ValidatorFactory.cpp
    Src/Domain/Program.cpp
    Src/Storage/Registry.cpp
    Src/Domain/Measurement/Sensor.cpp

    # Headers (не обязательны для сборки, но удобны для IDE)
    Src/CustomChart/ChartImageService.h
    Src/CustomChart/ChartManager.h
    Src/Domain/DeviceInitializer.h
    Src/Domain/Measurement/Sample.h
    Src/Domain/Measurement/Sensor.h
    Src/Domain/Measurement/TestDataBuffer.h
    Src/Domain/Mpi/Device.h
    Src/Domain/Mpi/Settings.h
    Src/Domain/Tests/AnalyzerFactory.h
    Src/Domain/Tests/Cyclic/Params.h
    Src/Domain/Tests/Cyclic/Regulatory/Algorithm.h
    Src/Domain/Tests/Cyclic/Regulatory/Analyzer.h
    Src/Domain/Tests/Cyclic/Regulatory/Params.h
    Src/Domain/Tests/Cyclic/Regulatory/Result.h
    Src/Domain/Tests/Cyclic/Regulatory/Runner.h
    Src/Domain/Tests/Cyclic/Shutoff/Algorithm.h
    Src/Domain/Tests/Cyclic/Shutoff/Analyzer.h
    Src/Domain/Tests/Cyclic/Shutoff/Params.h
    Src/Domain/Tests/Cyclic/Shutoff/Result.h
    Src/Domain/Tests/Cyclic/Shutoff/Runner.h
    Src/Domain/Tests/IAnalyzer.h
    Src/Domain/Tests/Main/Algorithm.h
    Src/Domain/Tests/Main/Analyzer.h
    Src/Domain/Tests/Main/Params.h
    Src/Domain/Tests/Main/Result.h
    Src/Domain/Tests/Main/Runner.h
    Src/Domain/Tests/Option/Algorithm.h
    Src/Domain/Tests/Option/Step/Algorithm.h
    Src/Domain/Tests/Option/Step/Analyzer.h
    Src/Domain/Tests/Option/Step/Params.h
    Src/Domain/Tests/Option/Step/Result.h
    Src/Domain/Tests/Option/Step/Runner.h
    Src/Domain/Tests/Stroke/Algorithm.h
    Src/Domain/Tests/Stroke/Analyzer.h
    Src/Domain/Tests/Stroke/Result.h
    Src/Domain/Tests/Stroke/Runner.h
    Src/Domain/Uart/Message.h
    Src/Domain/Uart/Port.h
    Src/Domain/Uart/Reader.h
    Src/Report/Builder.h
    Src/Report/BuilderFactory.h
    Src/Report/Patterns/B_CVT.h
    Src/Report/Patterns/B_SACVT.h
    Src/Report/Patterns/C_CVT.h
    Src/Report/Patterns/C_SACVT.h
    Src/Report/Patterns/C_SOVT.h
    Src/Report/Blocks/CyclicRanges.h
    Src/Report/Blocks/CyclicSummary.h
    Src/Report/Blocks/IBlock.h
    Src/Report/Blocks/ObjectInfo.h
    Src/Report/Blocks/SolenoidDetails.h
    Src/Report/Blocks/StepReaction.h
    Src/Report/Blocks/TechnicalResults.h
    Src/Report/Blocks/ValveSpec.h
    Src/Report/Saver.h
    Src/Report/Writer.h
    Src/Storage/AppSettings.h
    Src/Storage/SettingsGroup.h
    Src/Storage/Telemetry.h
    Src/Gui/MainWindow/CrossingIndicatorsPresenter.h
    Src/Gui/MainWindow/TelemetryUiMapper.h
    Src/Gui/MainWindow/TestController.h
    Src/Gui/Setup/ObjectWindow.h
    Src/Gui/Setup/ValveWindow/Logic.h
    Src/Gui/Setup/ValveWindow/ValveEnums.h
    Src/Gui/Setup/ValveWindow/ValveWindow.h
    Src/Gui/Setup/SelectTests.h
    Src/Gui/Setup/ValveWindow/Mapper.h
    Src/Gui/TestSettings/AbstractTestSettings.h
    Src/Gui/TestSettings/CyclicTestSettings.h
    Src/Gui/TestSettings/OtherTestSettings.h
    Src/Gui/TestSettings/StepTestSettings.h
    Src/Gui/TestSettings/MainTestSettings.h
    Src/Gui/MainWindow/MainWindow.h
    Src/Domain/Program.h
    Src/Storage/Registry.h
    Src/CustomChart/MyChart.h
    Src/CustomChart/MySeries.h
    Src/LabeledSlider/LabeledSlider.h
    Src/Domain/Tests/Test.h
    Src/Domain/Tests/BaseRunner.h
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.h
    Src/Domain/Tests/Option/Params.h
    Src/Domain/Tests/Option/Response/Runner.h
    Src/Domain/Tests/Option/Resolution/Runner.h
    Src/Utils/NumberUtils.h
    Src/Utils/Shortcuts/TabActionRouter.h
    Src/Utils/Shortcuts/TabBinder.h
    Src/Utils/SignalUtils.h
    Src/ValidatorFactory/RegexPatterns.h
    Src/ValidatorFactory/ValidatorFactory.h

    # UI forms
    Src/Gui/Setup/ObjectWindow.ui
    Src/Gui/Setup/SelectTests.ui
    Src/Gui/Setup/ValveWindow/ValveWindow.ui
    Src/Gui/MainWindow/MainWindow.ui
    Src/Gui/TestSettings/CyclicTestSettings.ui
    Src/Gui/TestSettings/MainTestSettings.ui
    Src/Gui/TestSettings/OtherTestSettings.ui
    Src/Gui/TestSettings/StepTestSettings.ui

    # Resources
    excel.qrc
    img.qrc
    translations.qrc
)



target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/CustomChart"
)

get_target_property(QT_CORE_INCDIR Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(QT_GUI_INCDIR Qt6::Gui INTERFACE_INCLUDE_DIRECTORIES)

# Берем первый путь из списка
list(GET QT_CORE_INCDIR 0 QT_CORE_INCLUDE_BASE)
list(GET QT_GUI_INCDIR 0 QT_GUI_INCLUDE_BASE)

set(QT_VER "${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}")

target_include_directories(QXlsx
    PRIVATE
        "${QT_CORE_INCLUDE_BASE}/${QT_VER}"
        "${QT_CORE_INCLUDE_BASE}/${QT_VER}/QtCore"
        "${QT_GUI_INCLUDE_BASE}/${QT_VER}"
        "${QT_GUI_INCLUDE_BASE}/${QT_VER}/QtGui"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::Charts
        QXlsx
)

# Для Visual Studio удобно задать стартовый проект
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# ------------------------------------------------------------
# Install
# ------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)