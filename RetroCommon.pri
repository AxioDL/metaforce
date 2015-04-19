INCLUDEPATH += $$PWD/include

include(../libSquish/libSquish.pri)

HEADERS += \
    $$PWD/include/RetroCommon.hpp

SOURCES += \
    $$PWD/src/RetroCommon.cpp \
    $$PWD/src/MREADecompress.cpp
