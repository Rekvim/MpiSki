CONFIG += c++17 console
QT += core testlib

ROOT_DIR = $$clean_path($$_PRO_FILE_PWD_/../..)
INCLUDEPATH += $$ROOT_DIR

SOURCES += \
    $$ROOT_DIR/Src/Domain/Tests/Cyclic/Regulatory/CyclicRegulatoryAnalyzer.cpp \
    $$ROOT_DIR/Src/Domain/Tests/Stroke/StrokeTestAnalyzer.cpp \
    analyzers/StrokeTestAnalyzerTest.cpp \
    main.cpp \
    analyzers/CyclicRegulatoryAnalyzerTest.cpp

# Заголовки тестов
HEADERS += \
    analyzers/CyclicRegulatoryAnalyzerTest.h \
    analyzers/SampleGenerator.h \
    analyzers/StrokeTestAnalyzerTest.h

# Для отладки
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g -O0
}

message("PRO_FILE_PWD = $$_PRO_FILE_PWD_")
message("ROOT_DIR = $$ROOT_DIR")
message("SOURCES ROOT = $$ROOT_DIR/Src")