HEADERS += \
    $$PWD/CPackedDatabase.hpp \
    $$PWD/CMemoryDatabase.hpp \
    $$PWD/CLooseDatabase.hpp \
    $$PWD/CSQLite.hpp \
    $$PWD/sqlite_hecl_vfs.h

SOURCES += \
    $$PWD/HECLDatabase.cpp \
    $$PWD/sqlite_hecl_mem_vfs.c \
    $$PWD/CRuntime.cpp \
    $$PWD/CProject.cpp
