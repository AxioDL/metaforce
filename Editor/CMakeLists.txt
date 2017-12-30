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
  if(WINDOWS_STORE)
      set(UWP_ASSETS
          platforms/win/Assets/LargeTile.scale-100.png
          platforms/win/Assets/LargeTile.scale-125.png
          platforms/win/Assets/LargeTile.scale-150.png
          platforms/win/Assets/LargeTile.scale-200.png
          platforms/win/Assets/LargeTile.scale-400.png
          platforms/win/Assets/SmallTile.scale-100.png
          platforms/win/Assets/SmallTile.scale-125.png
          platforms/win/Assets/SmallTile.scale-150.png
          platforms/win/Assets/SmallTile.scale-200.png
          platforms/win/Assets/SmallTile.scale-400.png
          platforms/win/Assets/SplashScreen.scale-100.png
          platforms/win/Assets/SplashScreen.scale-125.png
          platforms/win/Assets/SplashScreen.scale-150.png
          platforms/win/Assets/SplashScreen.scale-200.png
          platforms/win/Assets/SplashScreen.scale-400.png
          platforms/win/Assets/Square44x44Logo.scale-100.png
          platforms/win/Assets/Square44x44Logo.scale-125.png
          platforms/win/Assets/Square44x44Logo.scale-150.png
          platforms/win/Assets/Square44x44Logo.scale-200.png
          platforms/win/Assets/Square44x44Logo.scale-400.png
          platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-16.png
          platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-24.png
          platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-32.png
          platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-48.png
          platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-256.png
          platforms/win/Assets/Square150x150Logo.scale-100.png
          platforms/win/Assets/Square150x150Logo.scale-125.png
          platforms/win/Assets/Square150x150Logo.scale-150.png
          platforms/win/Assets/Square150x150Logo.scale-200.png
          platforms/win/Assets/Square150x150Logo.scale-400.png
          platforms/win/Assets/urde.scale-100.png
          platforms/win/Assets/urde.scale-125.png
          platforms/win/Assets/urde.scale-150.png
          platforms/win/Assets/urde.scale-200.png
          platforms/win/Assets/urde.scale-400.png
          platforms/win/Assets/WideTile.scale-100.png
          platforms/win/Assets/WideTile.scale-125.png
          platforms/win/Assets/WideTile.scale-150.png
          platforms/win/Assets/WideTile.scale-200.png
          platforms/win/Assets/WideTile.scale-400.png)
      set_property(SOURCE platforms/win/Package.appxmanifest PROPERTY VS_DEPLOYMENT_CONTENT 1)
      set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_CONTENT 1)
      set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
      list(APPEND PLAT_SRCS ${UWP_ASSETS} platforms/win/Package.appxmanifest)
  endif()
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

set_target_properties(urde PROPERTIES COMPILE_DEFINITIONS URDE_DLPACKAGE="${URDE_DLPACKAGE}")

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
if(COMMAND add_sanitizers)
  add_sanitizers(urde)
endif()
if(COMMAND cotire)
  cotire(urde)
endif()

set_target_properties(urde PROPERTIES
                      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/mac/Info.plist"
                      MACOSX_BUNDLE_BUNDLE_NAME "urde")

if (NOT WINDOWS_STORE)
  add_dependencies(urde visigen hecl)
  add_custom_command(TARGET urde POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:visigen> $<TARGET_FILE_DIR:urde>)
  add_custom_command(TARGET urde POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:visigen> $<TARGET_FILE_DIR:hecl>)
  add_custom_command(TARGET urde POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hecl> $<TARGET_FILE_DIR:urde>)
else()
  set_property(TARGET urde PROPERTY VS_WINRT_COMPONENT TRUE)
  # This should match the Package.appxmanifest
  set_property(TARGET urde PROPERTY VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.14393.0")
endif()
