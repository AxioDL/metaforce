TARGET = hecl
CONFIG -= Qt
QT      =
LIBS   -= -lQtGui -lQtCore
unix:QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
unix:!macx:LIBS += -std=c++11 -stdlib=libc++ -lc++abi

LIBPATH += $$OUT_PWD/../lib
LIBS += -lhecl

SOURCES += \
    $$PWD/main.cpp

HEADERS += \
    COptionsParser.hpp \
    CDriver.hpp

