add_executable(mkwmicon mkwmicon.c)
target_link_libraries(mkwmicon ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
target_include_directories(mkwmicon PRIVATE ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

macro(declare_wmicon_target)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/Runtime/platforms/freedesktop/mainicon_netwm.bin
            COMMAND $<TARGET_FILE:mkwmicon>
            ARGS ${CMAKE_BINARY_DIR}/Runtime/platforms/freedesktop/mainicon_netwm.bin
            DEPENDS
            ${CMAKE_SOURCE_DIR}/Runtime/platforms/freedesktop/128x128/apps/metaforce.png
            ${CMAKE_SOURCE_DIR}/Runtime/platforms/freedesktop/64x64/apps/metaforce.png
            ${CMAKE_SOURCE_DIR}/Runtime/platforms/freedesktop/48x48/apps/metaforce.png
            ${CMAKE_SOURCE_DIR}/Runtime/platforms/freedesktop/32x32/apps/metaforce.png
            ${CMAKE_SOURCE_DIR}/Runtime/platforms/freedesktop/16x16/apps/metaforce.png
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Runtime/platforms/freedesktop
            COMMENT "Generating mainicon_netwm.bin")
    bintoc(mainicon_netwm.cpp ${CMAKE_BINARY_DIR}/Runtime/platforms/freedesktop/mainicon_netwm.bin MAINICON_NETWM)
endmacro()
