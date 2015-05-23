TEMPLATE = lib
CONFIG += staticlib
TARGET = hecl-blender
CONFIG -= Qt
QT      =
unix:QMAKE_CXXFLAGS += -std=c++11
unix:QMAKE_CFLAGS += -std=c99
unix:LIBS += -std=c++11
clang:QMAKE_CXXFLAGS += -stdlib=libc++
clang:LIBS += -stdlib=libc++ -lc++abi

HEADERS += \
    $$PWD/CBlenderConnection.hpp

SOURCES += \
    $$PWD/CBlenderConnection.cpp

DISTFILES += \
    $$PWD/blendershell.py \
    $$PWD/addon/__init__.py \
    $$PWD/addon/hmdl/__init__.py \
    $$PWD/addon/hmdl/hmdl_anim.py \
    $$PWD/addon/hmdl/hmdl_mesh.py \
    $$PWD/addon/hmdl/hmdl_shader.py \
    $$PWD/addon/hmdl/hmdl_skin.py \
    $$PWD/addon/hmdl/hmdl_txtr.py

