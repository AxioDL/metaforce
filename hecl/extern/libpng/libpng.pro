TEMPLATE = lib
CONFIG += staticlib
TARGET = png
CONFIG -= Qt
QT      =

HEADERS += \
    png.h \
    pngconf.h \
    pngdebug.h \
    pnginfo.h \
    pngpriv.h \
    pngstruct.h \
    pnglibconf.h
    
SOURCES += \
    png.c \
    pngerror.c \
    pngget.c \
    pngmem.c \
    pngpread.c \
    pngread.c \
    pngrio.c \
    pngrtran.c \
    pngrutil.c \
    pngset.c \
    pngtrans.c \
    pngwio.c \
    pngwrite.c \
    pngwtran.c \
    pngwutil.c

