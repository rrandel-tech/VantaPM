cmake_minimum_required(VERSION 3.25)
project(VantaPM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(qtermwidget6 REQUIRED)

set(SOURCES
        Source/Main.cpp
        Source/MainWindow.cpp
        Source/MainWindow.hpp
        Source/UI/Pages/SearchPage.cpp
        Source/UI/Pages/SearchPage.hpp
        Source/UI/Widgets/Terminal.cpp
        Source/UI/Widgets/Terminal.hpp
        Source/UI/Widgets/TerminalWidget.cpp
        Source/UI/Widgets/TerminalWidget.hpp
        Source/SettingsManager.cpp
        Source/SettingsManager.hpp
        Source/UI/SettingsDialog.cpp
        Source/UI/SettingsDialog.hpp
)

qt_add_executable(VantaPM
        ${SOURCES}
        Resources/resources.qrc
)

target_link_libraries(VantaPM PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        qtermwidget6
)

target_include_directories(VantaPM PRIVATE Source)