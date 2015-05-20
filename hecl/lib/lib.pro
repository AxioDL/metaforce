TEMPLATE = lib
CONFIG += staticlib
TARGET = hecl
CONFIG -= Qt
QT      =
unix:QMAKE_CXXFLAGS += -std=c++11
unix:QMAKE_CFLAGS += -std=c99
unix:LIBS += -std=c++11
clang:QMAKE_CXXFLAGS += -stdlib=libc++
clang:LIBS += -stdlib=libc++ -lc++abi

INCLUDEPATH += $$PWD ../include ../extern

include (frontend/frontend.pri)
include (backend/backend.pri)
include (database/database.pri)
include (runtime/runtime.pri)
