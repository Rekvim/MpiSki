CONFIG += c++17 console
QT += core testlib

ROOT_DIR = $$clean_path($$_PRO_FILE_PWD_/../..)
INCLUDEPATH += $$ROOT_DIR

SOURCES += \
    $$ROOT_DIR/Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.cpp \
    $$ROOT_DIR/Src/Domain/Tests/Option/Step/StepTestAnalyzer.cpp \
    $$ROOT_DIR/Src/Domain/Tests/Stroke/StrokeTestAnalyzer.cpp \
    analyzers/StepTestAnalyzerTest.cpp \
    analyzers/StrokeTestAnalyzerTest.cpp \
    main.cpp \
    analyzers/CyclicRegulatoryAnalyzerTest.cpp

# Заголовки тестов
HEADERS += \
    analyzers/CyclicRegulatoryAnalyzerTest.h \
    analyzers/SampleGenerator.h \
    analyzers/StepTestAnalyzerTest.h \
    analyzers/StrokeTestAnalyzerTest.h

# Для отладки
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g -O0
}