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
    $$PWD/addon/hmdl/HMDLMesh.py \
    $$PWD/addon/hmdl/HMDLShader.py \
    $$PWD/addon/hmdl/HMDLSkin.py \
    $$PWD/addon/hmdl/HMDLTxtr.py \
    $$PWD/addon/sact/__init__.py \
    $$PWD/addon/sact/SACTAction.py \
    $$PWD/addon/sact/SACTEvent.py \
    $$PWD/addon/sact/SACTSubtype.py \
    $$PWD/addon/sact/ANIM.py

