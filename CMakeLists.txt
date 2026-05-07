cmake_minimum_required(VERSION 3.21)

project(MpiSki VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-ftime-report)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    SerialPort
    Charts
)

qt_standard_project_setup()

add_subdirectory(QXlsx)
add_subdirectory(Src)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    excel.qrc
    img.qrc
    translations.qrc
)

set_target_properties(MpiSki PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Gui
        Qt6::Widgets
)

option(MPISKI_BUILD_TESTS "Build tests" OFF)

if(MPISKI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)