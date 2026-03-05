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

        Source/SettingsManager.cpp
        Source/SettingsManager.hpp

        Source/UI/SettingsDialog.cpp
        Source/UI/SettingsDialog.hpp

        Source/UI/Pages/SearchPage.cpp
        Source/UI/Pages/SearchPage.hpp

        Source/UI/Pages/InstalledPage.cpp
        Source/UI/Pages/InstalledPage.hpp

        Source/UI/Pages/UpdatePage.cpp
        Source/UI/Pages/UpdatePage.hpp

        Source/UI/Pages/MaintenancePage.cpp
        Source/UI/Pages/MaintenancePage.hpp

        Source/UI/Pages/FlatpakPage.cpp
        Source/UI/Pages/FlatpakPage.hpp

        Source/UI/Pages/RepositoryPage.cpp
        Source/UI/Pages/RepositoryPage.hpp

        Source/UI/Pages/KernelPage.cpp
        Source/UI/Pages/KernelPage.hpp

        Source/UI/UpdateConfirmDialog.cpp
        Source/UI/UpdateConfirmDialog.hpp

        Source/UI/KernelInstallDialog.cpp
        Source/UI/KernelInstallDialog.hpp

        Source/Backend/Package.hpp
        Source/Backend/PacmanParser.cpp
        Source/Backend/PacmanParser.hpp
        Source/Backend/PacmanBackend.cpp
        Source/Backend/PacmanBackend.hpp
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