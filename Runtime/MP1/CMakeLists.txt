include_directories(. ../Common)
add_executable(mp1
               main.cpp)
target_link_libraries(mp1
                      RuntimeCommonCharacter
                      RuntimeCommon)
