TARGET = hecl
CONFIG -= Qt
QT      =
LIBS   -= -lQtGui -lQtCore
unix:QMAKE_CXXFLAGS += -std=c++11
unix:LIBS += -std=c++11
clang:QMAKE_CXXFLAGS += -stdlib=libc++
clang:LIBS += -stdlib=libc++ -lc++abi

INCLUDEPATH += ../include ../extern/Athena/include ../extern

LIBPATH += $$OUT_PWD/../lib \
    $$OUT_PWD/../dataspec \
    $$OUT_PWD/../blender \
    $$OUT_PWD/../extern/blowfish \
    $$OUT_PWD/../extern/libpng

LIBS += -lhecl -lhecl-dataspec -lhecl-blender -lblowfish -lpng

SOURCES += \
    $$PWD/main.cpp

HEADERS += \
    ToolBase.hpp \
    ToolPackage.hpp \
    ToolExtract.hpp \
    ToolInit.hpp \
    ToolHelp.hpp \
    ToolGroup.hpp \
    ToolCook.hpp \
    ToolClean.hpp \
    ToolAdd.hpp \
    ToolRemove.hpp \
    ToolSpec.hpp

