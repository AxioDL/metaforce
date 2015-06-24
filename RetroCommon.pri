INCLUDEPATH += $$PWD/DNA $$PWD/include

include(../libSquish/libSquish.pri)
include(PakLib/PakLib.pri)

HEADERS += \
    $$PWD/include/RetroCommon.hpp

SOURCES += \
    $$PWD/src/RetroCommon.cpp \
    $$PWD/src/MREADecompress.cpp

include(DNA/DNA.pri)
