cmake_minimum_required(VERSION 3.20)

# ── Project ──────────────────────────────────────────────────────────────────
project(ThunderFighter
    VERSION     0.1.0
    DESCRIPTION "雷霆战机 — A desktop 2D space shooter (QGraphicsView, pure C++)"
    LANGUAGES   CXX
)

# ── C++ Standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD       20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS     OFF)

# ── Qt MOC / RCC ────────────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── vcpkg ─────────────────────────────────────────────────────────────────────
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "" FORCE)
if(DEFINED ENV{VCPKG_ROOT} AND NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# ── Output ────────────────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Qt5 (纯 C++，无 Quick/Qml) ──────────────────────────────────────────────
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

# ═════════════════════════════════════════════════════════════════════════════
#  ① Common Agent — 不依赖任何人
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_common OBJECT
    src/common/MathUtils.cpp
    src/common/Logger.cpp
    src/common/Geometry.cpp
    src/common/AirMap.cpp
)
target_include_directories(thf_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ═════════════════════════════════════════════════════════════════════════════
#  ② ViewModel Agent — 取消 Model 层，数据即可绘制对象
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_viewmodel OBJECT
    src/viewmodel/SpiritVM.cpp
    src/viewmodel/GameMapVM.cpp
    src/viewmodel/Player.cpp
    src/viewmodel/Enemy.cpp
    src/viewmodel/Bullet.cpp
    src/viewmodel/CollisionSystem.cpp
    src/viewmodel/ScoreManager.cpp
    include/viewmodel/SpiritVM.hpp
    include/viewmodel/GameMapVM.hpp
)
target_include_directories(thf_viewmodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_viewmodel PUBLIC thf_common Qt5::Core Qt5::Gui)

# ═════════════════════════════════════════════════════════════════════════════
#  ③ View Agent — 纯 C++ QGraphicsView
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_view OBJECT
    src/view/GameView.cpp
    src/view/GameScene.cpp
    src/view/PlayerItem.cpp
    src/view/EnemyItem.cpp
    src/view/BulletItem.cpp
    src/view/StarFieldItem.cpp
    src/view/HudOverlay.cpp
    src/view/StartScreen.cpp
    src/view/ModeSelectScreen.cpp
    src/view/GameOverScreen.cpp
    src/view/PauseOverlay.cpp
    include/view/GameView.hpp
    include/view/GameScene.hpp
    include/view/HudOverlay.hpp
    include/view/StartScreen.hpp
    include/view/ModeSelectScreen.hpp
    include/view/GameOverScreen.hpp
    include/view/PauseOverlay.hpp
    include/view/PlayerItem.hpp
    include/view/EnemyItem.hpp
    include/view/BulletItem.hpp
    include/view/StarFieldItem.hpp
)
target_include_directories(thf_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_view PUBLIC thf_common Qt5::Gui Qt5::Widgets)
# ⚠️ 不链接 thf_viewmodel — 编译隔离确保 View 绝不 include ViewModel 头文件

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
#  QRC 资源
# ═════════════════════════════════════════════════════════════════════════════
qt5_add_resources(QRC_SOURCES resources/resources.qrc)

# ═════════════════════════════════════════════════════════════════════════════
#  ⑤ 主可执行文件
# ═════════════════════════════════════════════════════════════════════════════
add_executable(${PROJECT_NAME}
    src/app/main.cpp
    src/app/AppAgent.cpp
    ${QRC_SOURCES}
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
    $<TARGET_OBJECTS:thf_view>
    $<TARGET_OBJECTS:thf_resource>
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    thf_common thf_viewmodel thf_view thf_resource
    Qt5::Core Qt5::Gui Qt5::Widgets
)

# ── Warnings ──────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Debug 平台插件修复 ──────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND VCPKG_INSTALLED_DIR)
    set(_plugin_src "${VCPKG_INSTALLED_DIR}/x64-mingw-dynamic/debug/plugins/platforms/qwindows.dll")
    set(_plugin_dst "${CMAKE_BINARY_DIR}/bin/plugins/platforms/qwindows.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_plugin_src}" "${_plugin_dst}"
        COMMENT "Copying debug qwindows.dll platform plugin"
    )
endif()

# ═════════════════════════════════════════════════════════════════════════════
#  ⑥ Tests — 依赖 Common + ViewModel
# ═════════════════════════════════════════════════════════════════════════════
find_package(Catch2 CONFIG QUIET)
if(Catch2_FOUND)
    add_executable(${PROJECT_NAME}_tests
        tests/test_main.cpp
        tests/test_player.cpp
        tests/test_collision.cpp
        tests/test_game_map_vm.cpp
        $<TARGET_OBJECTS:thf_common>
        $<TARGET_OBJECTS:thf_viewmodel>
    )
    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        thf_common thf_viewmodel
        Qt5::Core
        Catch2::Catch2WithMain
    )
    include(CTest)
    include(Catch)
    catch_discover_tests(${PROJECT_NAME}_tests)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
