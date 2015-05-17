TARGET = hecl
CONFIG -= Qt
QT      =
LIBS   -= -lQtGui -lQtCore
unix:QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
unix:!macx:LIBS += -std=c++11 -stdlib=libc++ -lc++abi

INCLUDEPATH += ../include

LIBPATH += $$OUT_PWD/../lib $$OUT_PWD/../extern/sqlite3
LIBS += -lhecl -lsqlite3

SOURCES += \
    $$PWD/main.cpp

HEADERS += \
    COptionsParser.hpp \
    CDriver.hpp

