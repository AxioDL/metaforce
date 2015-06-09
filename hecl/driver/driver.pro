TARGET = hecl
CONFIG -= Qt
QT      =
LIBS   -= -lQtGui -lQtCore
unix:QMAKE_CXXFLAGS += -std=c++11
unix:LIBS += -std=c++11
clang:QMAKE_CXXFLAGS += -stdlib=libc++
clang:LIBS += -stdlib=libc++ -lc++abi

INCLUDEPATH += ../include

LIBPATH += $$OUT_PWD/../lib \
    $$OUT_PWD/../dataspec \
    $$OUT_PWD/../blender \
    $$OUT_PWD/../extern/blowfish \
    $$OUT_PWD/../extern/libpng

LIBS += -lhecl -lhecl-dataspec -lhecl-blender -lblowfish -lpng

# Yay!! Athena IO
include(../extern/Athena/AthenaCore.pri)

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
    CToolAdd.hpp \
    CToolRemove.hpp \
    CToolSpec.hpp

