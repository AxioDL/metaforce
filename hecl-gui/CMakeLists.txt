cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)

add_executable(hecl-gui
    MainWindow.ui MainWindow.hpp MainWindow.cpp
    FileDirDialog.hpp FileDirDialog.cpp
    main.cpp)

target_link_libraries(hecl-gui ${Qt5Widgets_LIBRARIES})
