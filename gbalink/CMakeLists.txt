if (NOT WIN32)
include_directories(. ./../Runtime ${ATHENA_INCLUDE_DIR})
add_executable(gbalink main.cpp)
endif()
