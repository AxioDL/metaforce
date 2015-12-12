include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(locale)

atdna(atdna_Space.cpp Space.hpp)

set(SPACE_HEADERS
    ResourceOutliner/ResourceOutliner.hpp)

set(SPACE_SOURCES
    ResourceOutliner/ResourceOutliner.cpp)

add_executable(rude WIN32
               main.cpp
               Space.hpp Space.cpp atdna_Space.cpp
               ${SPACE_HEADERS} ${SPACE_SOURCES}
               ViewManager.hpp ViewManager.cpp)

target_link_libraries(rude
                      RudeLocales
                      RuntimeCommonCharacter
                      RuntimeCommonInput
                      RuntimeCommon
                      DNAMP3 DNAMP2 DNAMP1
                      DNACommon Specter SpecterFonts freetype
                      HECLDatabase HECLBlender HECLRuntime HECLCommon AthenaCore NOD
                      LogVisor AthenaLibYaml Boo ${PNG_LIB} squish xxhash Math
                      ${ZLIB_LIBRARIES} ${LZO_LIB}
                      ${BOO_SYS_LIBS})
