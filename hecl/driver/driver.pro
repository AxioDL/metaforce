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
    $$OUT_PWD/../extern/blowfish \
    $$OUT_PWD/../extern/libpng \
    $$OUT_PWD/../extern/Athena/lib \
    $$OUT_PWD/../extern/RetroCommon/DataSpec \
    $$OUT_PWD/../extern/RetroCommon/NODLib/lib

LIBS += -lhecl -lhecl-blender -lblowfish -lpng -lpthread
LIBS += -Wl,--whole-archive -lRetroDataSpec -Wl,--no-whole-archive -lNOD -lLogVisor
CONFIG(debug, debug|release) {
    LIBS += -lAthena-d
} else {
    LIBS += -lAthena
}

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

