add_executable(heclTest WIN32 main.cpp)
target_link_libraries(heclTest
    HECLDatabase HECLRuntime HECLBackend HECLFrontend HECLBlender HECLCommon AthenaCore xxhash
    LogVisor Boo ${ZLIB_LIBRARIES} ${BOO_SYS_LIBS})
