cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)
find_package(Qt5Network)

if(APPLE)
  set(PLAT_SRCS MacOSSystemVersion.hpp MacOSSystemVersion.mm)
  find_library(FOUNDATION_LIBRARY Foundation)
  set(PLAT_LIBS ${FOUNDATION_LIBRARY})
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

add_executable(hecl-gui WIN32 MACOSX_BUNDLE
    MainWindow.ui MainWindow.hpp MainWindow.cpp
    FileDirDialog.hpp ErrorLabel.hpp
    SysReqTableView.hpp SysReqTableView.cpp
    VectorISATableView.hpp VectorISATableView.cpp
    VectorISATableModel.hpp VectorISATableModelIntel.hpp
    DownloadManager.hpp DownloadManager.cpp
    Common.hpp Common.cpp ${PLAT_SRCS} main.cpp)

set_target_properties(hecl-gui PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/mac/Info.plist")

target_link_libraries(hecl-gui ${PLAT_LIBS} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES} zeus)
