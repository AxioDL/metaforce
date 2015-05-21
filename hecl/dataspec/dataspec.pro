TEMPLATE = lib
CONFIG += staticlib
TARGET = dataspec
CONFIG -= Qt
QT      =
unix:QMAKE_CXXFLAGS += -std=c++11
unix:QMAKE_CFLAGS += -std=c99
unix:LIBS += -std=c++11
clang:QMAKE_CXXFLAGS += -stdlib=libc++
clang:LIBS += -stdlib=libc++ -lc++abi

INCLUDEPATH += $$PWD ../include ../extern

HEADERS += \
    DUMB.hpp \
    HMDL.hpp \
    MATR.hpp \
    STRG.hpp \
    TXTR.hpp

SOURCES += \
    dataspec.cpp

