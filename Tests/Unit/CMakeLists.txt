cmake_minimum_required(VERSION 3.21)

project(AnalyzerTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test)

# Корень проекта относительно папки с этим CMakeLists.txt
get_filename_component(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

add_executable(AnalyzerTests
    ${ROOT_DIR}/Src/Domain/Tests/Cyclic/Regulatory/Analyzer.cpp
    ${ROOT_DIR}/Src/Domain/Tests/Option/Step/Analyzer.cpp
    ${ROOT_DIR}/Src/Domain/Tests/Stroke/Analyzer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/StepTestAnalyzerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/StrokeTestAnalyzerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/CyclicRegulatoryAnalyzerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/CyclicRegulatoryAnalyzerTest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/SampleGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/StepTestAnalyzerTest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/analyzers/StrokeTestAnalyzerTest.h
)

target_include_directories(AnalyzerTests
    PRIVATE
        ${ROOT_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(AnalyzerTests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(AnalyzerTests PRIVATE -g -O0)
endif()

enable_testing()
add_test(NAME AnalyzerTests COMMAND AnalyzerTests)