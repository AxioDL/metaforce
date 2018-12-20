add_executable(heclTest WIN32 main.cpp)
target_link_libraries(heclTest
    ${HECL_APPLICATION_REPS_TARGETS_LIST}
    hecl-full hecl-blender-addon
    athena-core athena-libyaml xxhash logvisor boo
    ${ZLIB_LIBRARIES} lzokay ${BOO_SYS_LIBS})
if(COMMAND add_sanitizers)
  add_sanitizers(heclTest)
endif()
