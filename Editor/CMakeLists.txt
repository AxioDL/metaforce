include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/DataSpec ${CMAKE_SOURCE_DIR}/Runtime)
add_subdirectory(locale)
add_subdirectory(icons)
add_subdirectory(badging)

atdna(atdna_Space.cpp Space.hpp)
atdna(atdna_ResourceBrowser.cpp ResourceBrowser.hpp)
atdna(atdna_ModelViewer.cpp ModelViewer.hpp)
atdna(atdna_ParticleEditor.cpp ParticleEditor.hpp)
atdna(atdna_InformationCenter.cpp InformationCenter.hpp)
atdna(atdna_GameMode.cpp GameMode.hpp)

if(WIN32)
  set(PLAT_SRCS platforms/win/urde.rc)
elseif(APPLE)
  set(PLAT_SRCS platforms/mac/mainicon.icns)
  set_source_files_properties(platforms/mac/mainicon.icns PROPERTIES
                              MACOSX_PACKAGE_LOCATION Resources)
elseif(UNIX)
  add_subdirectory(platforms/freedesktop)
  declare_wmicon_target()
  set(PLAT_SRCS mainicon_netwm.c)
endif()

add_executable(urde WIN32 MACOSX_BUNDLE
               main.cpp ${PLAT_SRCS}
               Space.hpp Space.cpp atdna_Space.cpp
               SplashScreen.hpp SplashScreen.cpp
               ResourceBrowser.hpp ResourceBrowser.cpp atdna_ResourceBrowser.cpp
               ModelViewer.hpp ModelViewer.cpp atdna_ModelViewer.cpp
               ParticleEditor.hpp ParticleEditor.cpp atdna_ParticleEditor.cpp
               InformationCenter.hpp InformationCenter.hpp atdna_InformationCenter.cpp
               ProjectManager.hpp ProjectManager.cpp
               ViewManager.hpp ViewManager.cpp
               Resource.hpp Resource.cpp
               Camera.hpp Camera.cpp
               GameMode.hpp GameMode.cpp atdna_GameMode.cpp)

target_link_libraries(urde
                      UrdeLocales
                      UrdeIcons
                      UrdeBadging
                      RuntimeCommon
                      specter specter-fonts freetype ${DATA_SPEC_LIBS}
                      hecl-common hecl-blender-addon
                      athena-core nod logvisor athena-libyaml amuse boo
                      ${PNG_LIB} libjpeg-turbo squish xxhash zeus
                      kabufuda jbus ${ZLIB_LIBRARIES} ${LZO_LIB}
                      ${BOO_SYS_LIBS})

set_target_properties(urde PROPERTIES
                      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/mac/Info.plist")

add_dependencies(urde visigen hecl)

add_custom_command(TARGET urde POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:visigen> $<TARGET_FILE_DIR:urde>)
add_custom_command(TARGET urde POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hecl> $<TARGET_FILE_DIR:urde>)