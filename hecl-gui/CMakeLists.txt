cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)
find_package(Qt5Network)
find_package(Qt5Xml)

if(APPLE)
  set(PLAT_SRCS MacOSSystemVersion.hpp MacOSSystemVersion.mm)
  set_source_files_properties(MacOSSystemVersion.mm
                              PROPERTIES COMPILE_FLAGS -fobjc-arc)
  find_library(FOUNDATION_LIBRARY Foundation)
  set(PLAT_LIBS ${FOUNDATION_LIBRARY})
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(PLAT_LIBS pthread dl)
endif()

if(WIN32)
  list(APPEND PLAT_SRCS platforms/win/hecl-gui.rc platforms/win/hecl-gui.manifest)
elseif(APPLE)
  list(APPEND PLAT_SRCS platforms/mac/mainicon.icns)
  set_source_files_properties(platforms/mac/mainicon.icns PROPERTIES
          MACOSX_PACKAGE_LOCATION Resources)
endif()

add_subdirectory(platforms/freedesktop)
declare_qticon_target()
list(APPEND PLAT_SRCS mainicon_qt.cpp)

file(GLOB QUAZIP_SRCS quazip/quazip/*.c quazip/quazip/*.cpp quazip/quazip/*.h)
list(REMOVE_ITEM QUAZIP_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/quazip/quazip/quagzipfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quazip/quazip/quagzipfile.h)
include_directories(quazip/quazip)
add_definitions(-DQUAZIP_STATIC=1)

add_executable(hecl-gui WIN32 MACOSX_BUNDLE
    MainWindow.ui MainWindow.hpp MainWindow.cpp
    LaunchMenu.hpp LaunchMenu.cpp
    EscapeSequenceParser.hpp EscapeSequenceParser.cpp
    FileDirDialog.hpp ErrorLabel.hpp
    SysReqTableView.hpp SysReqTableView.cpp
    VectorISATableView.hpp VectorISATableView.cpp
    VectorISATableModel.hpp VectorISATableModelIntel.hpp
    FindBlender.hpp FindBlender.cpp
    DownloadManager.hpp DownloadManager.cpp
    ExtractZip.hpp ExtractZip.cpp ${QUAZIP_SRCS}
    Common.hpp Common.cpp ${PLAT_SRCS} main.cpp)
if(COMMAND add_sanitizers)
  add_sanitizers(hecl-gui)
endif()

set_target_properties(hecl-gui PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/mac/Info.plist")

target_link_libraries(hecl-gui ${PLAT_LIBS}
                      ${Qt5Widgets_LIBRARIES}
                      ${Qt5Network_LIBRARIES}
                      ${Qt5Xml_LIBRARIES}
                      hecl-light logvisor zeus athena-core athena-libyaml xxhash z)
