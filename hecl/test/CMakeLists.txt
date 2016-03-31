add_executable(heclTest WIN32 main.cpp)
target_link_libraries(heclTest
    hecl-database hecl-runtime hecl-backend hecl-frontend
    hecl-hmdl hecl-blender hecl-common athena-core xxhash
    logvisor boo ${ZLIB_LIBRARIES} ${BOO_SYS_LIBS})
