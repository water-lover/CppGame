cmake_minimum_required(VERSION 3.20)

# ── Project ──────────────────────────────────────────────────────────────────
project(CppGame
    VERSION     0.1.0
    DESCRIPTION "A desktop game built with C++"
    LANGUAGES   CXX
)

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD       20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS     OFF)

# ── vcpkg integration (via VCPKG_ROOT or CMAKE_TOOLCHAIN_FILE) ────────────────
# If you set VCPKG_ROOT before running CMake, the toolchain is picked up
# automatically.  You can also pass it on the command line:
#   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
if(DEFINED ENV{VCPKG_ROOT} AND NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# ── Output directories ────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Dependencies (from vcpkg) ────────────────────────────────────────────────
find_package(SFML   COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
find_package(Catch2 CONFIG QUIET)

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.hpp
    src/*.h
)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

# ── Include directories ──────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ── Link libraries ──────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Network
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
)

# ── Warnings ──────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# ── Tests (optional) ─────────────────────────────────────────────────────────
if(Catch2_FOUND)
    set(TEST_SOURCES
        tests/test_main.cpp
        tests/test_game_model.cpp
        src/Model/GameModel.cpp
    )

    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        Catch2::Catch2WithMain
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(${PROJECT_NAME}_tests)
endif()
