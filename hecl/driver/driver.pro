TARGET = hecl
CONFIG -= Qt
QT      =
LIBS   -= -lQtGui -lQtCore
unix:QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
unix:!macx:LIBS += -std=c++11 -stdlib=libc++ -lc++abi

INCLUDEPATH += ../include

LIBPATH += $$OUT_PWD/../lib \
    $$OUT_PWD/../extern/sqlite3 \
    $$OUT_PWD/../extern/blowfish \
    $$OUT_PWD/../extern/libpng \
    $$OUT_PWD/../extern/zlib

LIBS += -lhecl -lsqlite3 -lblowfish -lpng -lz

SOURCES += \
    $$PWD/main.cpp

HEADERS += \
    CToolBase.hpp \
    CToolPackage.hpp \
    CToolInit.hpp \
    CToolHelp.hpp \
    CToolGroup.hpp \
    CToolCook.hpp \
    CToolClean.hpp \
    CToolAdd.hpp

