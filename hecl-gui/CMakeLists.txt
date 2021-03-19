cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if (APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
    set(QT_HOMEBREW_PATH /usr/local/opt/qt)
elseif (APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL arm64)
    set(QT_HOMEBREW_PATH /opt/homebrew/opt/qt)
else ()
    set(QT_HOMEBREW_PATH "")
endif ()

find_package(Qt6Widgets QUIET PATHS ${QT_HOMEBREW_PATH})
if (Qt6Widgets_FOUND)
    find_package(Qt6 COMPONENTS Core5Compat Network Widgets Xml Gui REQUIRED PATHS ${QT_HOMEBREW_PATH})
    set(QUAZIP_QT_MAJOR_VERSION 6 CACHE STRING "")
else ()
    find_package(Qt5Widgets REQUIRED PATHS ${QT_HOMEBREW_PATH})
    find_package(Qt5 COMPONENTS Network Widgets Xml Gui REQUIRED PATHS ${QT_HOMEBREW_PATH})
    set(QUAZIP_QT_MAJOR_VERSION 5 CACHE STRING "")
endif ()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(QUAZIP_INSTALL OFF CACHE BOOL "")
add_subdirectory(quazip)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64)
    set(VectorISATableModel_HEADER VectorISATableModelIntel.hpp)
else ()
    set(VectorISATableModel_HEADER "")
endif ()

add_executable(hecl-gui WIN32 MACOSX_BUNDLE
        #ArgumentEditor.cpp
        #ArgumentEditor.hpp
        #ArgumentEditor.ui
        Common.cpp
        Common.hpp
        #CVarDialog.cpp
        #CVarDialog.hpp
        #CVarDialog.ui
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
        MainWindow.cpp
        MainWindow.hpp
        MainWindow.ui
        LayerDialog.cpp
        LayerDialog.hpp
        LayerDialog.ui
        SysReqTableView.cpp
        SysReqTableView.hpp
        VectorISATableModel.hpp
        ${VectorISATableModel_HEADER}
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

if (Qt6Widgets_FOUND)
    set(Qt_LIBS
            Qt6::Core
            Qt6::Core5Compat
            Qt6::Gui
            Qt6::Network
            Qt6::Widgets
            Qt6::Xml)
else ()
    set(Qt_LIBS
            Qt5::Core
            Qt5::Gui
            Qt5::Network
            Qt5::Widgets
            Qt5::Xml)
    # Check for static linking
    if (WIN32 AND NOT "$<TARGET_PROPERTY:hecl-gui,MSVC_RUNTIME_LIBRARY>" MATCHES "DLL$")
        list(APPEND Qt_LIBS Qt5::QWindowsIntegrationPlugin)
    endif ()
endif ()
target_link_libraries(hecl-gui PRIVATE
        ${Qt_LIBS}
        hecl-light
        zeus
        QuaZip::QuaZip)

target_include_directories(hecl-gui PRIVATE quazip/quazip)
target_compile_definitions(hecl-gui PRIVATE QUAZIP_STATIC=1)

if (APPLE)
    target_sources(hecl-gui PRIVATE
            MacOSSystemVersion.hpp
            MacOSSystemVersion.mm
            )
    set_source_files_properties(MacOSSystemVersion.mm
            PROPERTIES COMPILE_FLAGS -fobjc-arc
            )
    find_library(FOUNDATION_LIBRARY Foundation)
    target_link_libraries(hecl-gui PRIVATE ${FOUNDATION_LIBRARY})
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(hecl-gui PRIVATE
            dl
            pthread
            )
endif ()

if (WIN32)
    target_sources(hecl-gui PRIVATE
            platforms/win/hecl-gui.manifest
            platforms/win/hecl-gui.rc
            )
    target_link_libraries(hecl-gui PRIVATE
            Version)
elseif (APPLE)
    set_target_properties(hecl-gui PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/mac/Info.plist"
            )
    target_sources(hecl-gui PRIVATE platforms/mac/mainicon.icns)
    set_source_files_properties(platforms/mac/mainicon.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
            )
    add_custom_command(
            TARGET hecl-gui POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:urde> $<TARGET_FILE_DIR:hecl-gui>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hecl> $<TARGET_FILE_DIR:hecl-gui>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:visigen> $<TARGET_FILE_DIR:hecl-gui>
            DEPENDS urde hecl visigen
    )
endif ()

add_subdirectory(platforms/freedesktop)
declare_qticon_target()
target_sources(hecl-gui PRIVATE mainicon_qt.cpp)

if (COMMAND add_sanitizers)
    add_sanitizers(hecl-gui)
endif ()

if (NOT MSVC)
    target_compile_options(hecl-gui PRIVATE -Wno-misleading-indentation)
endif ()
