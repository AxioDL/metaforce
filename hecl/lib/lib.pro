TEMPLATE = lib
CONFIG += staticlib
TARGET = hecl
CONFIG -= Qt
QT      =
unix:QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++

INCLUDEPATH += $$PWD ../include ../extern

include (frontend/frontend.pri)
include (backend/backend.pri)
include (database/database.pri)
include (runtime/runtime.pri)
