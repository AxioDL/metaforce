TEMPLATE = lib
CONFIG += staticlib c++11
TARGET = RetroDataSpec
CONFIG -= Qt
QT      =

# Get hecl's headers (when built as submodule in hecl's extern)
INCLUDEPATH += ../../../include \
    ../../Athena/include \
    ../../LogVisor/include \
    ../NODLib/include

include(DNACommon/DNACommon.pri)
include(DNAMP1/DNAMP1.pri)
include(DNAMP2/DNAMP2.pri)
include(DNAMP3/DNAMP3.pri)

HEADERS += \
    Logging.hpp \
    SpecBase.hpp

SOURCES += \
    SpecMP1.cpp \
    SpecMP2.cpp \
    SpecMP3.cpp \
    SpecBase.cpp
