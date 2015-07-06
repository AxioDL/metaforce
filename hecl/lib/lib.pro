TEMPLATE = lib
CONFIG += staticlib c++11
TARGET = hecl
CONFIG -= Qt
QT      =

INCLUDEPATH += $$PWD \
    ../include \
    ../extern \
    ../extern/Athena/include \
    ../extern/LogVisor/include

include (frontend/frontend.pri)
include (backend/backend.pri)
include (database/database.pri)
include (runtime/runtime.pri)

SOURCES += \
    HECL.cpp \
    ProjectPath.cpp \
    WideStringConvert.cpp
