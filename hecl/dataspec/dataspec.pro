TEMPLATE = lib
CONFIG += staticlib
TARGET = hecl-dataspec
CONFIG -= Qt
QT      =
unix:QMAKE_CXXFLAGS += -std=c++11
unix:QMAKE_CFLAGS += -std=c99
unix:LIBS += -std=c++11
clang:QMAKE_CXXFLAGS += -stdlib=libc++
clang:LIBS += -stdlib=libc++ -lc++abi

INCLUDEPATH += $$PWD ../include ../extern ../extern/Athena/include

HEADERS += \
    helpers.hpp \
    DUMB.hpp \
    HMDL.hpp \
    MATR.hpp \
    STRG.hpp \
    TXTR.hpp

SOURCES += \
    helpers.cpp \
    dataspec.cpp

