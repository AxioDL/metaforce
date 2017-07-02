add_executable(heclTest WIN32 main.cpp)
target_link_libraries(heclTest
    hecl-common hecl-blender-addon athena-core athena-libyaml xxhash
    logvisor boo ${ZLIB_LIBRARIES} ${LZO_LIB} ${BOO_SYS_LIBS})
