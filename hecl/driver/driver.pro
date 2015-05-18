TARGET = hecl
CONFIG -= Qt
QT      =
LIBS   -= -lQtGui -lQtCore
unix:QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
unix:!macx:LIBS += -std=c++11 -stdlib=libc++ -lc++abi

INCLUDEPATH += ../include

LIBPATH += $$OUT_PWD/../lib \
    $$OUT_PWD/../extern/sqlite3 \
    $$OUT_PWD/../extern/blowfish

LIBS += -lhecl -lsqlite3 -lblowfish

SOURCES += \
    $$PWD/main.cpp

HEADERS += \
    COptionsParser.hpp \
    CDriver.hpp

