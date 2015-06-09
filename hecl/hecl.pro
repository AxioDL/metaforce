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

HEADERS += \
    include/HECL.hpp \
    include/HECLBackend.hpp \
    include/HECLDatabase.hpp \
    include/HECLFrontend.hpp \
    include/HECLRuntime.hpp

SUBDIRS += \
    extern/blowfish \
    extern/libpng \
    extern/Athena \
    blender \
    lib \
    dataspec \
    driver

driver.depends = extern/blowfish
driver.depends = extern/libpng
driver.depends = blender
driver.depends = lib
driver.depends = dataspec
