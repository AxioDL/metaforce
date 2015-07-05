TEMPLATE = app
TARGET = hecl
CONFIG -= Qt
CONFIG += c++11
QT      =
LIBS   -= -lQtGui -lQtCore

INCLUDEPATH += ../include \
    ../extern/Athena/include \
    ../extern/LogVisor/include \
    ../extern

LIBPATH += $$OUT_PWD/../lib \
    $$OUT_PWD/../blender \
    $$OUT_PWD/../extern/LogVisor \
    $$OUT_PWD/../extern/RetroCommon/NODLib/lib \
    $$OUT_PWD/../extern/blowfish \
    $$OUT_PWD/../extern/libpng

LIBS += -lhecl -lhecl-blender -lblowfish -lNOD -lLogVisor -lpng -lpthread

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

