HEADERS += \
    $$PWD/IBackendFragEmitter.hpp \
    $$PWD/IBackendObject.hpp \
    $$PWD/IBackendSpec.hpp \
    $$PWD/IBackendVertEmitter.hpp

SOURCES += \
    $$PWD/GLSL/CGLSLFragEmitter.cpp \
    $$PWD/GLSL/CGLSLObject.cpp \
    $$PWD/GLSL/CGLSLSpec.cpp \
    $$PWD/GLSL/CGLSLVertEmitter.cpp \
    $$PWD/GX/CGXFragEmitter.cpp \
    $$PWD/GX/CGXObject.cpp \
    $$PWD/GX/CGXSpec.cpp \
    $$PWD/GX/CGXVertEmitter.cpp \
    $$PWD/HLSL/CHLSLFragEmitter.cpp \
    $$PWD/HLSL/CHLSLObject.cpp \
    $$PWD/HLSL/CHLSLSpec.cpp \
    $$PWD/HLSL/CHLSLVertEmitter.cpp \
    $$PWD/VSPIR/CVSPIRFragEmitter.cpp \
    $$PWD/VSPIR/CVSPIRObject.cpp \
    $$PWD/VSPIR/CVSPIRSpec.cpp \
    $$PWD/VSPIR/CVSPIRVertEmitter.cpp \
    $$PWD/HECLBackend.cpp

!isEmpty(LLVMROOT) {
    message("Added GX2 Backend")
SOURCES += \
    $$PWD/GX2LLVM/CGX2LLVMFragEmitter.cpp \
    $$PWD/GX2LLVM/CGX2LLVMObject.cpp \
    $$PWD/GX2LLVM/CGX2LLVMSpec.cpp \
    $$PWD/GX2LLVM/CGX2LLVMVertEmitter.cpp
}
