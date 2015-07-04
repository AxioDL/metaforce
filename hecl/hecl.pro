TEMPLATE = subdirs
CONFIG += c++11

# Enable building with LLVM dependencies
exists ($$PWD/llvm) {
    LLVMROOT = $$PWD/llvm
}
!isEmpty(LLVMROOT) {
    message("Configuring for LLVM backends using '$$LLVMROOT'")
    DEFINES += HECL_LLVM=1
}

# Make LogVisor headers global
LOGVISOR_INCLUDE = $$PWD/extern/LogVisor/include

HEADERS += \
    include/HECL/HECL.hpp \
    include/HECL/Backend.hpp \
    include/HECL/Database.hpp \
    include/HECL/Frontend.hpp \
    include/HECL/Runtime.hpp

SUBDIRS += \
    extern/LogVisor \
    extern/blowfish \
    extern/libpng \
    extern/libSquish \
    extern/Athena \
    extern/RetroCommon \
    blender \
    lib \
    driver

driver.depends = extern/blowfish
driver.depends = extern/libpng
driver.depends = extern/libSquish
driver.depends = extern/Athena
driver.depends = extern/RetroCommon
driver.depends = blender
driver.depends = lib
