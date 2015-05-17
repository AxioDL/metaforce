TEMPLATE = subdirs

# Enable building with LLVM dependencies
exists ($$PWD/llvm) {
    LLVMROOT = $$PWD/llvm
}
!isEmpty(LLVMROOT) {
    message("Configuring for LLVM backends using '$$LLVMROOT'")
    DEFINES += HECL_LLVM=1
}

HEADERS += \
    include/HECLBackend.hpp \
    include/HECLDatabase.hpp \
    include/HECLFrontend.hpp \
    include/HECLRuntime.hpp

SUBDIRS += \
    extern/sqlite3 \
    lib \
    driver

lib.depends = extern/sqlite3
driver.depends = lib
