cmake_minimum_required(VERSION 3.20)
project(SpeedCurveGUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    REQUIRED
)
find_package(Threads REQUIRED)

# 包含主项目的头文件
include_directories(include ../include)

# 编译主项目的源文件
set(LIB_SOURCES
    ../src/user_lib.cpp
    ../src/pid.cpp
    ../src/controller.cpp
    ../src/dji_motor.cpp
    ../src/can.cpp
)

set(GUI_SOURCES
    src/main.cpp
    src/main_window.cpp
    src/speed_monitor.cpp
    src/motor_data_manager.cpp
    src/simple_chart.cpp
    ${LIB_SOURCES}
)

set(GUI_HEADERS
    include/main_window.hpp
    include/speed_monitor.hpp
    include/motor_data_manager.hpp
    include/simple_chart.hpp
)

add_executable(${PROJECT_NAME} ${GUI_SOURCES} ${GUI_HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Threads::Threads
)

add_compile_options(-D__DEBUG__)

