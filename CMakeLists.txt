cmake_minimum_required(VERSION 3.20)

# ── Project ──────────────────────────────────────────────────────────────────
project(CppGame
    VERSION     0.1.0
    DESCRIPTION "A desktop 2D game built with Qt Quick and C++"
    LANGUAGES   CXX
)

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD       20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS     OFF)

# ── Qt MOC / UIC / RCC (auto-process Q_OBJECT macros) ──────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── vcpkg integration (manual install) ─────────────────────────────────────
# Dependencies are pre-installed via:
#   vcpkg install --triplet x64-mingw-dynamic --host-triplet x64-mingw-dynamic
# Disable manifest mode to avoid MSVC triplet conflicts.
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "" FORCE)
if(DEFINED ENV{VCPKG_ROOT} AND NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# ── Output directories ────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Qt5 (required: Quick + Multimedia for a game) ────────────────────────────
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    Gamepad
)

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.hpp
    src/*.h
    include/*.hpp
    include/*.h
)

# ── QML resource files ────────────────────────────────────────────────────
file(GLOB_RECURSE QML_FILES
    qml/*.qml
    qml/*.qmltypes
)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    ${SOURCES}
    resources/resources.qrc
)

# ── Include directories ──────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ── Link Qt5 libraries ─────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Quick
    Qt5::Qml
    Qt5::Gamepad
)

# ── Copy QML runtime files to output ────────────────────────────────────────
# Qt5's QML engine needs to find the QML files at runtime.
# In production, they are embedded via qrc. For development, we copy them.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/qml DESTINATION ${CMAKE_BINARY_DIR})

# ── Warnings ──────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# ── Tests (Catch2, optional) ─────────────────────────────────────────────────
find_package(Catch2 CONFIG QUIET)
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
