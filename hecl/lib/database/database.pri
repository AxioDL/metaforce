HEADERS += \
    $$PWD/CPackedDatabase.hpp \
    $$PWD/CMemoryDatabase.hpp \
    $$PWD/CLooseDatabase.hpp \
    $$PWD/CSQLite.hpp \
    $$PWD/sqlite_hecl_vfs.h

SOURCES += \
    $$PWD/HECLDatabase.cpp \
    $$PWD/CRuntime.cpp \
    $$PWD/CProject.cpp \
    $$PWD/sqlite_hecl_mem_vfs.c \
    $$PWD/sqlite_hecl_memlba_vfs.c \
    $$PWD/sqlite_hecl_memlba_make.c
