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

# ── Qt5 ──────────────────────────────────────────────────────────────────────
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Test)

# ═════════════════════════════════════════════════════════════════════════════
#  Common Agent
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_common OBJECT src/common/AirMap.cpp)
target_include_directories(thf_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ═════════════════════════════════════════════════════════════════════════════
#  ViewModel Agent
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_viewmodel OBJECT
    src/viewmodel/GameMapVM.cpp
    src/viewmodel/Player.cpp
    src/viewmodel/Enemy.cpp
    src/viewmodel/Bullet.cpp
    src/viewmodel/CollisionSystem.cpp
    src/viewmodel/ScoreManager.cpp
    src/viewmodel/AircraftStats.cpp
    src/viewmodel/SkillSystem.cpp
    src/viewmodel/WaveManager.cpp
    src/viewmodel/PowerUpManager.cpp
    src/viewmodel/Boss.cpp
    src/viewmodel/UpgradeManager.cpp
    src/viewmodel/SpiritVM.cpp
    src/viewmodel/MathUtils.cpp
    src/viewmodel/Geometry.cpp
    include/viewmodel/GameMapVM.hpp
    include/viewmodel/Player.hpp
    include/viewmodel/Enemy.hpp
    include/viewmodel/Bullet.hpp
    include/viewmodel/CollisionSystem.hpp
    include/viewmodel/ScoreManager.hpp
    include/viewmodel/AircraftStats.hpp
    include/viewmodel/SkillSystem.hpp
    include/viewmodel/WaveManager.hpp
    include/viewmodel/PowerUpManager.hpp
    include/viewmodel/Boss.hpp
    include/viewmodel/UpgradeManager.hpp
    include/viewmodel/SpiritVM.hpp
)
target_include_directories(thf_viewmodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_viewmodel PUBLIC thf_resource thf_common Qt5::Core Qt5::Gui)

# ═════════════════════════════════════════════════════════════════════════════
#  View Agent
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
    src/view/BossHealthBar.cpp
    src/view/LevelSelectScreen.cpp
    src/view/AircraftSelectScreen.cpp
    src/view/UpgradeScreen.cpp
    include/view/GameView.hpp
    include/view/GameScene.hpp
    include/view/PlayerItem.hpp
    include/view/EnemyItem.hpp
    include/view/BulletItem.hpp
    include/view/StarFieldItem.hpp
    include/view/HudOverlay.hpp
    include/view/StartScreen.hpp
    include/view/ModeSelectScreen.hpp
    include/view/GameOverScreen.hpp
    include/view/PauseOverlay.hpp
    include/view/BossHealthBar.hpp
    include/view/LevelSelectScreen.hpp
    include/view/AircraftSelectScreen.hpp
    include/view/UpgradeScreen.hpp
)
target_include_directories(thf_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_view PUBLIC thf_common Qt5::Gui Qt5::Widgets)

# ═════════════════════════════════════════════════════════════════════════════
#  Resource Agent
# ═════════════════════════════════════════════════════════════════════════════
add_library(thf_resource OBJECT
    src/resource/AssetManager.cpp
    src/resource/SaveManager.cpp
    src/resource/Logger.cpp
)
target_include_directories(thf_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_resource PUBLIC thf_common Qt5::Core Qt5::Gui)

# ═════════════════════════════════════════════════════════════════════════════
#  QRC
# ═════════════════════════════════════════════════════════════════════════════
qt5_add_resources(QRC_SOURCES resources/resources.qrc)

# ═════════════════════════════════════════════════════════════════════════════
#  Main executable
# ═════════════════════════════════════════════════════════════════════════════
add_executable(${PROJECT_NAME}
    src/app/main.cpp src/app/AppAgent.cpp ${QRC_SOURCES}
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
    $<TARGET_OBJECTS:thf_view>
    $<TARGET_OBJECTS:thf_resource>
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    thf_common thf_viewmodel thf_view thf_resource
    Qt5::Core Qt5::Gui Qt5::Widgets
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Debug plugin copy ──────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_plugin_search
        "${VCPKG_INSTALLED_DIR}/x64-mingw-dynamic/debug/plugins/platforms/qwindows.dll"
        "${CMAKE_SOURCE_DIR}/../vcpkg/packages/qt5-base_x64-mingw-dynamic/debug/plugins/platforms/qwindows.dll"
        "D:/vcpkg/packages/qt5-base_x64-mingw-dynamic/debug/plugins/platforms/qwindows.dll"
    )
    set(_plugin_found "")
    foreach(_p ${_plugin_search})
        if(EXISTS "${_p}")
            set(_plugin_found "${_p}")
            break()
        endif()
    endforeach()
    if(_plugin_found)
        set(_plugin_dst "${CMAKE_BINARY_DIR}/bin/plugins/platforms/qwindows.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_plugin_found}" "${_plugin_dst}"
            COMMENT "Copying debug qwindows.dll platform plugin"
        )
    endif()
endif()

# ═════════════════════════════════════════════════════════════════════════════
#  Tests
# ═════════════════════════════════════════════════════════════════════════════
find_package(Catch2 CONFIG QUIET)
if(Catch2_FOUND)
    add_executable(${PROJECT_NAME}_tests
        tests/test_main.cpp
        tests/test_player.cpp
        tests/test_collision.cpp
        tests/test_game_map_vm.cpp
        tests/test_aircraft_stats.cpp
        tests/test_skill_system.cpp
        tests/test_wave_manager.cpp
        tests/test_power_up.cpp
        tests/test_boss.cpp
        tests/test_upgrade_manager.cpp
        $<TARGET_OBJECTS:thf_common>
        $<TARGET_OBJECTS:thf_viewmodel>
        $<TARGET_OBJECTS:thf_resource>
    )
    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        thf_common thf_viewmodel thf_resource
        Qt5::Core
        Catch2::Catch2WithMain
    )

    # ── 自动化测试（QTest 鼠标模拟，带 View 层） ─────────
    add_executable(${PROJECT_NAME}_auto
        tests/test_auto_play.cpp
        ${QRC_SOURCES}
        $<TARGET_OBJECTS:thf_common>
        $<TARGET_OBJECTS:thf_viewmodel>
        $<TARGET_OBJECTS:thf_resource>
        $<TARGET_OBJECTS:thf_view>
    )
    target_include_directories(${PROJECT_NAME}_auto PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${PROJECT_NAME}_auto PRIVATE
        thf_common thf_viewmodel thf_resource thf_view
        Catch2::Catch2WithMain
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Test
    )

    include(CTest)
    include(Catch)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
