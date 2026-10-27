cmake_minimum_required(VERSION 3.20)
project(CS449PegSolitaire VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

add_library(solitaire_core
    src/Board.cpp
    src/BoardFactory.cpp
    src/SolitaireGame.cpp
)
target_include_directories(solitaire_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(solitaire_gui
    src/main.cpp
    src/SolitaireWindow.cpp
    src/SolitaireWindow.h
)
target_link_libraries(solitaire_gui PRIVATE solitaire_core Qt6::Widgets)

add_executable(solitaire_tests
    tests/BoardTests.cpp
    tests/SolitaireGameTests.cpp
)
target_link_libraries(solitaire_tests PRIVATE solitaire_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(solitaire_tests)

# Keep all Sprint 0 targets available but separate from production Solitaire.
add_subdirectory(sprint_0)
