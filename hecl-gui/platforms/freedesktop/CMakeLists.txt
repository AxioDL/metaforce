include_directories(${LIBPNG_INCLUDE_DIR})
add_executable(mkqticon mkqticon.c)
target_link_libraries(mkqticon ${PNG_LIB} ${ZLIB_LIBRARIES})

macro(declare_qticon_target)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/hecl-gui/platforms/freedesktop/mainicon_qt.bin
                   COMMAND $<TARGET_FILE:mkqticon>
                   ARGS ${CMAKE_BINARY_DIR}/hecl-gui/platforms/freedesktop/mainicon_qt.bin
                   DEPENDS
                   ${CMAKE_SOURCE_DIR}/hecl-gui/platforms/freedesktop/128x128/apps/hecl.png
                   ${CMAKE_SOURCE_DIR}/hecl-gui/platforms/freedesktop/64x64/apps/hecl.png
                   ${CMAKE_SOURCE_DIR}/hecl-gui/platforms/freedesktop/48x48/apps/hecl.png
                   ${CMAKE_SOURCE_DIR}/hecl-gui/platforms/freedesktop/32x32/apps/hecl.png
                   ${CMAKE_SOURCE_DIR}/hecl-gui/platforms/freedesktop/16x16/apps/hecl.png
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hecl-gui/platforms/freedesktop
                   COMMENT "Generating mainicon_qt.bin")
bintoc(mainicon_qt.cpp ${CMAKE_BINARY_DIR}/hecl-gui/platforms/freedesktop/mainicon_qt.bin MAINICON_QT)
endmacro()
