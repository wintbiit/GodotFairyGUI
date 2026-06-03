cmake_minimum_required(VERSION 3.22)

project(GodotFairyGUI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(GODOT_CPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE)
set(GODOT_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(godot-cpp)

file(GLOB_RECURSE GODOT_FAIRYGUI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(godot_fairygui SHARED ${GODOT_FAIRYGUI_SOURCES})

target_include_directories(godot_fairygui
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(godot_fairygui PRIVATE godot::cpp)

if(MSVC)
    target_compile_options(godot_fairygui PRIVATE /W4 /permissive-)
else()
    target_compile_options(godot_fairygui PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(godot_fairygui PROPERTIES
    OUTPUT_NAME "godot_fairygui"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/example/addons/godot_fairygui/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
)
