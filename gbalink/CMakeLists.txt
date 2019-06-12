add_executable(gbalink main.cpp)
target_link_libraries(gbalink jbus)
target_include_directories(gbalink PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../Runtime ${ATHENA_INCLUDE_DIR})
