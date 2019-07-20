add_executable(genlocales genlocales.cpp)
target_link_libraries(genlocales fmt athena-core)

set(LOCALES_IN en_US.yaml en_GB.yaml ja_JP.yaml)
set(LOCALES_OUT ${CMAKE_CURRENT_BINARY_DIR}/locales-inl.hpp)
add_custom_command(OUTPUT ${LOCALES_OUT} COMMAND $<TARGET_FILE:genlocales>
                   ARGS ${LOCALES_OUT} ${LOCALES_IN}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   DEPENDS genlocales ${LOCALES_IN})

add_library(UrdeLocales ${LOCALES_OUT} locale.hpp locale.cpp)
target_link_libraries(UrdeLocales fmt)
target_include_directories(UrdeLocales PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(specter PUBLIC UrdeLocales)
