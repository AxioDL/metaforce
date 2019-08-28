cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Network Widgets Xml REQUIRED)

file(GLOB QUAZIP_SRCS quazip/quazip/*.c quazip/quazip/*.cpp quazip/quazip/*.h)
list(REMOVE_ITEM QUAZIP_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/quazip/quazip/quagzipfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/quazip/quazip/quagzipfile.h
)
if (NOT MSVC)
  set_source_files_properties(${QUAZIP_SRCS} PROPERTIES COMPILE_OPTIONS -Wno-deprecated-declarations)
endif()

add_executable(hecl-gui WIN32 MACOSX_BUNDLE
  ArgumentEditor.cpp
  ArgumentEditor.hpp
  ArgumentEditor.ui
  Common.cpp
  Common.hpp
  DownloadManager.cpp
  DownloadManager.hpp
  ErrorLabel.hpp
  EscapeSequenceParser.cpp
  EscapeSequenceParser.hpp
  ExtractZip.cpp
  ExtractZip.hpp
  FileDirDialog.hpp
  FindBlender.cpp
  FindBlender.hpp
  LaunchMenu.cpp
  LaunchMenu.hpp
  MainWindow.cpp
  MainWindow.hpp
  MainWindow.ui
  SysReqTableView.cpp
  SysReqTableView.hpp
  VectorISATableModel.hpp
  VectorISATableModelIntel.hpp
  VectorISATableView.cpp
  VectorISATableView.hpp

  main.cpp

  ${QUAZIP_SRCS}
)

target_compile_definitions(hecl-gui PRIVATE
  # Disable implicit conversions from ASCII to QString.
  -DQT_NO_CAST_FROM_ASCII
  -DQT_NO_CAST_TO_ASCII

  # Disable implicit conversions of QByteArray to const char* or const void*
  -DQT_NO_CAST_FROM_BYTEARRAY

  # Disable narrowing conversions in signal/slot connect() calls.
  -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT

  # Disable unsafe overloads of QProcess' start() function.
  -DQT_NO_PROCESS_COMBINED_ARGUMENT_START

  # Disable implicit QString->QUrl conversions to enforce use of proper resolving functions.
  -DQT_NO_URL_CAST_FROM_STRING

  # Allows for more efficient string concatenation, resulting in less temporaries.
  -DQT_USE_QSTRINGBUILDER 
)

target_link_libraries(hecl-gui PRIVATE
  Qt5::Network
  Qt5::Widgets
  Qt5::Xml

  athena-core
  hecl-light
  logvisor
  xxhash
  z
  zeus
)

target_include_directories(hecl-gui PRIVATE quazip/quazip)
target_compile_definitions(hecl-gui PRIVATE QUAZIP_STATIC=1)

if(APPLE)
  target_sources(hecl-gui PRIVATE
    MacOSSystemVersion.hpp
    MacOSSystemVersion.mm
  )
  set_source_files_properties(MacOSSystemVersion.mm
    PROPERTIES COMPILE_FLAGS -fobjc-arc
  )
  find_library(FOUNDATION_LIBRARY Foundation)
  target_link_libraries(hecl-gui PRIVATE ${FOUNDATION_LIBRARY})
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  target_link_libraries(hecl-gui PRIVATE
    dl
    pthread
  )
endif()

if(WIN32)
  target_sources(hecl-gui PRIVATE
    platforms/win/hecl-gui.manifest
    platforms/win/hecl-gui.rc
  )
elseif(APPLE)
  target_sources(hecl-gui PRIVATE platforms/mac/mainicon.icns)
  set_source_files_properties(platforms/mac/mainicon.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
  )
endif()

add_subdirectory(platforms/freedesktop)
declare_qticon_target()
target_sources(hecl-gui PRIVATE mainicon_qt.cpp)

if(COMMAND add_sanitizers)
  add_sanitizers(hecl-gui)
endif()

if (NOT MSVC)
  target_compile_options(hecl-gui PRIVATE -Wno-misleading-indentation)
endif()

set_target_properties(hecl-gui PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/mac/Info.plist"
)
