cmake_minimum_required(VERSION 3.16)
project(FocusForest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets QUIET)
if(NOT Qt5_FOUND)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(NOT Qt6_FOUND)
        message(FATAL_ERROR "Qt5 or Qt6 with Widgets module is required.")
    endif()
endif()

file(GLOB_RECURSE SOURCES src/*.cpp)
list(APPEND SOURCES main.cpp)

add_executable(FocusForest ${SOURCES})

target_include_directories(FocusForest PRIVATE src)

if(Qt5_FOUND)
    target_link_libraries(FocusForest PRIVATE Qt5::Widgets)
else()
    target_link_libraries(FocusForest PRIVATE Qt6::Widgets)
endif()

# --- Tests (optional, requires Qt5 Test) ---
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    find_package(Qt5 COMPONENTS Test QUIET)
    if(Qt5_FOUND)
        file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
        add_executable(FocusForestTests ${TEST_SOURCES})
        target_include_directories(FocusForestTests PRIVATE src)
        target_link_libraries(FocusForestTests PRIVATE Qt5::Test)
        enable_testing()
        add_test(NAME FocusForestTests COMMAND FocusForestTests)
    endif()
endif()
