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

INCLUDEPATH += $$PWD ../include ../extern \
../extern/Athena/include
../extern/RetroCommon/include

include(../extern/RetroCommon/RetroCommon.pri)

include(hecl/hecl.pri)
include(mp1/mp1.pri)
include(mp2/mp2.pri)
include(mp3/mp3.pri)

HEADERS += \
    helpers.hpp

SOURCES += \
    helpers.cpp \
    dataspec.cpp

