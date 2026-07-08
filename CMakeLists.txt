cmake_minimum_required(VERSION 3.20)

# ── Project ──────────────────────────────────────────────────────────────────
project(ThunderFighter
    VERSION     0.1.0
    DESCRIPTION "雷霆战机 — A desktop 2D space shooter built with Qt Quick and C++"
    LANGUAGES   CXX
)

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD       20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS     OFF)

# ── Qt MOC / UIC / RCC (auto-process Q_OBJECT macros) ──────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── vcpkg integration ────────────────────────────────────────────────────────
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "" FORCE)
if(DEFINED ENV{VCPKG_ROOT} AND NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# ── Output directories ────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Qt5 ───────────────────────────────────────────────────────────────────────
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
)

# ═════════════════════════════════════════════════════════════════════════════
#  ① Common Agent — 不依赖任何人
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_common OBJECT
    src/common/MathUtils.cpp
    src/common/Logger.cpp
    src/common/Geometry.cpp
)
target_include_directories(thf_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 没有 target_link_libraries — Common 不依赖任何人

# ═════════════════════════════════════════════════════════════════════════════
#  ② Logic Agent — 依赖 Common
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_logic OBJECT
    src/logic/Player.cpp
    src/logic/Enemy.cpp
    src/logic/Bullet.cpp
    src/logic/CollisionSystem.cpp
    src/logic/ScoreManager.cpp
    src/logic/GameModel.cpp
    src/logic/GameViewModel.cpp
    include/logic/GameViewModel.hpp
)
target_include_directories(thf_logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_logic PUBLIC thf_common Qt5::Core)

# ═════════════════════════════════════════════════════════════════════════════
#  ③ View Agent — 依赖 Common + Logic（QML 三绑定，不 C++ include）
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_view OBJECT
)
target_include_directories(thf_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_view PUBLIC thf_common Qt5::Quick Qt5::Qml Qt5::Gui)

# ═════════════════════════════════════════════════════════════════════════════
#  ④ Resource Agent — 依赖 Common
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_resource OBJECT
    src/resource/AssetManager.cpp
    src/resource/SaveManager.cpp
)
target_include_directories(thf_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_resource PUBLIC thf_common Qt5::Core Qt5::Gui)

# ═════════════════════════════════════════════════════════════════════════════
#  QML 资源文件
# ═════════════════════════════════════════════════════════════════════════════
qt5_add_resources(QRC_SOURCES resources/resources.qrc)

# ═════════════════════════════════════════════════════════════════════════════
#  ⑤ 主可执行文件 — 链接所有 OBJECT 库
# ═════════════════════════════════════════════════════════════════════════════
add_executable(${PROJECT_NAME}
    src/app/main.cpp
    src/app/AppAgent.cpp
    ${QRC_SOURCES}
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_logic>
    $<TARGET_OBJECTS:thf_view>
    $<TARGET_OBJECTS:thf_resource>
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    thf_common
    thf_logic
    Qt5::Core Qt5::Gui Qt5::Quick Qt5::Qml
)

# ── Warnings ──────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Debug 构建：复制 debug 版 Qt 平台插件 ────────────────────────────────
# Debug 构建时 Qt DLL 从 vcpkg debug/ 部署，但平台插件默认取 release 版，
# 导致 qwindows.dll 依赖的 libfreetype.dll 不匹配。
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND VCPKG_INSTALLED_DIR)
    set(_plugin_src "${VCPKG_INSTALLED_DIR}/x64-mingw-dynamic/debug/plugins/platforms/qwindows.dll")
    set(_plugin_dst "${CMAKE_BINARY_DIR}/bin/plugins/platforms/qwindows.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_plugin_src}" "${_plugin_dst}"
        COMMENT "Copying debug qwindows.dll platform plugin"
    )
endif()

# ═════════════════════════════════════════════════════════════════════════════
#  ⑥ Tests — 依赖 Common + Logic
# ═════════════════════════════════════════════════════════════════════════════
find_package(Catch2 CONFIG QUIET)
if(Catch2_FOUND)
    add_executable(${PROJECT_NAME}_tests
        tests/test_main.cpp
        tests/test_game_model.cpp
        tests/test_player.cpp
        tests/test_collision.cpp
        $<TARGET_OBJECTS:thf_common>
        $<TARGET_OBJECTS:thf_logic>
    )
    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        thf_common thf_logic
        Qt5::Core
        Catch2::Catch2WithMain
    )
    include(CTest)
    include(Catch)
    catch_discover_tests(${PROJECT_NAME}_tests)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
