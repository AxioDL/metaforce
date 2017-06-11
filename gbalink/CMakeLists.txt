include_directories(. ./../Runtime ${ATHENA_INCLUDE_DIR} ${JBUS_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl")
add_executable(gbalink main.cpp ../hecl/extern/boo/logvisor/lib/logvisor.cpp ../hecl/lib/hecl.cpp)
target_link_libraries(gbalink jbus)
