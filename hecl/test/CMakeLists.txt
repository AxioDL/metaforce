add_executable(heclTest WIN32 main.cpp)
target_link_libraries(heclTest
    HECLDatabase HECLBackend HECLFrontend HECLBlender HECLCommon AthenaCore
    LogVisor Boo ${BOO_SYS_LIBS})
