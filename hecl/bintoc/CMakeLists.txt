add_executable(bintoc bintoc.c)
macro(bintoc out in sym)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${out}
                     COMMAND $<TARGET_FILE:bintoc> ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${in} ${CMAKE_CURRENT_BINARY_DIR}/${out} ${sym}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${in})
endmacro()
