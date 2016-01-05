include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(locale)

atdna(atdna_Space.cpp Space.hpp)
atdna(atdna_ResourceOutliner.cpp ResourceOutliner.hpp)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  add_definitions("-DRUDE_BINARY_CONFIGS=1")
endif()

add_executable(rude WIN32
               main.cpp
               Space.hpp Space.cpp atdna_Space.cpp
               SplashScreen.hpp SplashScreen.cpp
               ResourceOutliner.hpp ResourceOutliner.cpp atdna_ResourceOutliner.cpp
               ProjectManager.hpp ProjectManager.cpp
               ViewManager.hpp ViewManager.cpp)

target_link_libraries(rude
                      RudeLocales
                      RuntimeCommonCharacter
                      RuntimeCommonInput
                      RuntimeCommon
                      DNAMP3 DNAMP2 DNAMP1
                      DNACommon Specter SpecterFonts freetype ${DATA_SPEC_LIBS}
                      HECLDatabase HECLBackend HECLFrontend HECLBlender HECLRuntime HECLCommon AthenaCore NOD
                      LogVisor AthenaLibYaml Boo ${PNG_LIB} squish xxhash Math
                      ${ZLIB_LIBRARIES} ${LZO_LIB}
                      ${BOO_SYS_LIBS})
