cmake_minimum_required(VERSION 3.16)
project(YourTestSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Отдельные директории для сгенерированных файлов
set(CMAKE_AUTOMOC_MOC_DIR ${CMAKE_CURRENT_BINARY_DIR}/moc)
set(CMAKE_AUTORCC_RCC_DIR ${CMAKE_CURRENT_BINARY_DIR}/rcc)
set(CMAKE_AUTOUIC_UI_DIR ${CMAKE_CURRENT_BINARY_DIR}/ui)

# Решение проблемы с одинаковыми именами файлов
set(CMAKE_CXX_OBJECT_PATH_SEP "_")
set_property(GLOBAL PROPERTY TARGET_OBJECT_PATH_MAX 256)

# Поиск Qt6 с приватными модулями
find_package(Qt6 REQUIRED COMPONENTS
    Core
    CorePrivate      # Добавьте это
    Gui
    GuiPrivate       # Добавьте это
    Widgets
    SerialPort
    Charts
)

# Затем в target_link_libraries добавьте:
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::CorePrivate    # Добавьте это
    Qt6::Gui
    Qt6::GuiPrivate     # Добавьте это
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Charts
    QXlsx
)

# ============= ПРОСТОЙ СПОСОБ ПОДКЛЮЧЕНИЯ QXlsx =============
# Вместо add_subdirectory, просто добавляем исходники QXlsx напрямую
set(QXLSX_HEADERPATH ${CMAKE_CURRENT_SOURCE_DIR}/QXlsx/header)
set(QXLSX_SOURCEPATH ${CMAKE_CURRENT_SOURCE_DIR}/QXlsx/source)

# Проверяем существование папок
if(EXISTS ${QXLSX_SOURCEPATH} AND EXISTS ${QXLSX_HEADERPATH})
    message(STATUS "Found QXlsx at ${QXLSX_SOURCEPATH}")

    # Собираем все .cpp файлы QXlsx
    file(GLOB_RECURSE QXLSX_SOURCES "${QXLSX_SOURCEPATH}/*.cpp")
    file(GLOB_RECURSE QXLSX_HEADERS "${QXLSX_HEADERPATH}/*.h")

    message(STATUS "Found ${CMAKE_CURRENT_SOURCE_DIR}/QXlsx")
else()
    message(WARNING "QXlsx not found, building without Excel support")
    set(QXLSX_SOURCES "")
    set(QXLSX_HEADERS "")
endif()

# ВАШИ ИСХОДНЫЕ ФАЙЛЫ - замените на полный список из вашего .pro файла
set(SOURCES
    main.cpp
    Src/CustomChart/ChartImageService.cpp
    Src/CustomChart/ChartManager.cpp
    Src/Domain/DeviceInitializer.cpp
    Src/Domain/Mpi/Device.cpp
    Src/Domain/Mpi/Settings.cpp
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAlgorithm.cpp
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAnalyzer.cpp
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.cpp
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.cpp
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAnalyzer.cpp
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffRunner.cpp
    Src/Domain/Tests/Main/MainAlgorithm.cpp
    Src/Domain/Tests/Main/MainAnalyzer.cpp
    Src/Domain/Tests/Main/MainRunner.cpp
    Src/Domain/Tests/Option/OptionAlgorithm.cpp
    Src/Domain/Tests/Option/Step/StepAlgorithm.cpp
    Src/Domain/Tests/Option/Step/StepAnalyzer.cpp
    Src/Domain/Tests/Option/Step/StepRunner.cpp
    Src/Domain/Tests/Stroke/StrokeAlgorithm.cpp
    Src/Domain/Tests/Stroke/StrokeAnalyzer.cpp
    Src/Domain/Tests/Stroke/StrokeRunner.cpp
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
    Src/Ui/MainWindow/CrossingIndicatorsPresenter.cpp
    Src/Ui/MainWindow/TelemetryUiMapper.cpp
    Src/Ui/MainWindow/TestController.cpp
    Src/Ui/Setup/ValveWindow/Logic.cpp
    Src/Ui/Setup/ValveWindow/Mapper.cpp
    Src/Utils/NumberUtils.cpp
    Src/Ui/Setup/ObjectWindow.cpp
    Src/Ui/Setup/ValveWindow/ValveWindow.cpp
    Src/Ui/Setup/SelectTests.cpp
    Src/Ui/MainWindow/MainWindow.cpp
    Src/Ui/TestSettings/CyclicTestSettings.cpp
    Src/Ui/TestSettings/OtherTestSettings.cpp
    Src/Ui/TestSettings/StepTestSettings.cpp
    Src/Ui/TestSettings/MainTestSettings.cpp
    Src/CustomChart/MyChart.cpp
    Src/CustomChart/MySeries.cpp
    Src/LabeledSlider/LabeledSlider.cpp
    Src/Domain/Tests/BaseRunner.cpp
    Src/Domain/Tests/Test.cpp
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp
    Src/Domain/Tests/Option/Response/ResponseRunner.cpp
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.cpp
    Src/ValidatorFactory/ValidatorFactory.cpp
    Src/Domain/Program.cpp
    Src/Storage/Registry.cpp
    Src/Domain/Sensor.cpp
    ${QXLSX_SOURCES}  # Добавляем исходники QXlsx
)

set(HEADERS
    Src/CustomChart/ChartImageService.h
    Src/CustomChart/ChartManager.h
    Src/Domain/DeviceInitializer.h
    Src/Domain/Measurement/Sample.h
    Src/Domain/Measurement/TestDataBuffer.h
    Src/Domain/Mpi/Device.h
    Src/Domain/Mpi/Settings.h
    Src/Domain/Tests/AnalyzerFactory.h
    Src/Domain/Tests/Cyclic/CyclicParams.h
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAlgorithm.h
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryAnalyzer.h
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryParams.h
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryResult.h
    Src/Domain/Tests/Cyclic/Regulatory/RegulatoryRunner.h
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAlgorithm.h
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffAnalyzer.h
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffParams.h
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffResult.h
    Src/Domain/Tests/Cyclic/Shutoff/ShutoffRunner.h
    Src/Domain/Tests/IAnalyzer.h
    Src/Domain/Tests/Main/MainAlgorithm.h
    Src/Domain/Tests/Main/MainAnalyzer.h
    Src/Domain/Tests/Main/MainParams.h
    Src/Domain/Tests/Main/MainResult.h
    Src/Domain/Tests/Main/MainRunner.h
    Src/Domain/Tests/Option/OptionAlgorithm.h
    Src/Domain/Tests/Option/Step/StepAlgorithm.h
    Src/Domain/Tests/Option/Step/StepAnalyzer.h
    Src/Domain/Tests/Option/Step/StepParams.h
    Src/Domain/Tests/Option/Step/StepResult.h
    Src/Domain/Tests/Option/Step/StepRunner.h
    Src/Domain/Tests/Stroke/StrokeAlgorithm.h
    Src/Domain/Tests/Stroke/StrokeAnalyzer.h
    Src/Domain/Tests/Stroke/StrokeResult.h
    Src/Domain/Tests/Stroke/StrokeRunner.h
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
    Src/Ui/MainWindow/CrossingIndicatorsPresenter.h
    Src/Ui/MainWindow/TelemetryUiMapper.h
    Src/Ui/MainWindow/TestController.h
    Src/Ui/Setup/ObjectWindow.h
    Src/Ui/Setup/ValveWindow/Logic.h
    Src/Ui/Setup/ValveWindow/ValveEnums.h
    Src/Ui/Setup/ValveWindow/ValveWindow.h
    Src/Ui/Setup/SelectTests.h
    Src/Ui/Setup/ValveWindow/Mapper.h
    Src/Ui/TestSettings/AbstractTestSettings.h
    Src/Ui/TestSettings/CyclicTestSettings.h
    Src/Ui/TestSettings/OtherTestSettings.h
    Src/Ui/TestSettings/StepTestSettings.h
    Src/Ui/TestSettings/MainTestSettings.h
    Src/Ui/MainWindow/MainWindow.h
    Src/Domain/Program.h
    Src/Storage/Registry.h
    Src/Domain/Sensor.h
    Src/CustomChart/MyChart.h
    Src/CustomChart/MySeries.h
    Src/LabeledSlider/LabeledSlider.h
    Src/Domain/Tests/Test.h
    Src/Domain/Tests/BaseRunner.h
    Src/Domain/Tests/CyclicPositioner/CyclicTestPositioner.h
    Src/Domain/Tests/Option/OptionTestParams.h
    Src/Domain/Tests/Option/Response/ResponseRunner.h
    Src/Domain/Tests/Option/Resolution/ResolutionRunner.h
    Src/Utils/NumberUtils.h
    Src/Utils/Shortcuts/TabActionRouter.h
    Src/Utils/Shortcuts/TabBinder.h
    Src/Utils/SignalUtils.h
    Src/ValidatorFactory/RegexPatterns.h
    Src/ValidatorFactory/ValidatorFactory.h
    ${QXLSX_HEADERS}
)

set(UI_FORMS
    Src/Ui/Setup/ObjectWindow.ui
    Src/Ui/Setup/SelectTests.ui
    Src/Ui/Setup/ValveWindow/ValveWindow.ui
    Src/Ui/MainWindow/MainWindow.ui
    Src/Ui/TestSettings/CyclicTestSettings.ui
    Src/Ui/TestSettings/MainTestSettings.ui
    Src/Ui/TestSettings/OtherTestSettings.ui
    Src/Ui/TestSettings/StepTestSettings.ui
)

set(RESOURCES
    excel.qrc
    img.qrc
    translations.qrc
)

# Добавляем пути для include
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/CustomChart
    ${QXLSX_HEADERPATH}  # Путь к заголовкам QXlsx
)

# Определяем макросы для QXlsx
add_definitions(-DQXLSX_LIBRARY)

# Создаем исполняемый файл
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FORMS}
    ${RESOURCES}
)

# Линкуем библиотеки
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Charts
)

# Настройки для правильного маппинга объектных файлов
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC TRUE
    AUTORCC TRUE
    AUTOUIC TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    set_property(TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD OFF)
endif()