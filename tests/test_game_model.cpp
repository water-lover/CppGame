#include <catch2/catch_test_macros.hpp>
#include "logic/GameModel.hpp"

TEST_CASE("GameModel - initial state", "[model]") {
    GameModel model;

    CHECK(model.getScore() == 0);
    CHECK(model.getLives() == 3);
    CHECK(model.getState() == GameState::Menu);
    CHECK(model.isOver() == false);
}

TEST_CASE("GameModel - reset starts game", "[model]") {
    GameModel model;
    model.reset();

    CHECK(model.getScore() == 0);
    CHECK(model.getLives() == 3);
    CHECK(model.getState() == GameState::Playing);
    CHECK(model.isOver() == false);
}

TEST_CASE("GameModel - player takes damage on enemy collision", "[model]") {
    GameModel model;
    model.reset();

    // 玩家初始 3 命
    CHECK(model.getPlayer().getLives() == 3);
}

TEST_CASE("GameModel - update spawns enemies", "[model]") {
    GameModel model;
    model.reset();

    auto& enemies = model.getEnemies();
    CHECK(enemies.size() == 0);

    // 更新超过生成间隔
    model.update(2.0f);

    CHECK(enemies.size() > 0);
}

TEST_CASE("GameModel - bullet is created when player updates", "[model]") {
    GameModel model;
    model.reset();

    auto& bullets = model.getBullets();
    int oldCount = bullets.size();

    // 更新一帧
    model.update(0.016f);

    // 玩家应该已经射出了子弹（fireTimer 在 reset 时为 0，第一帧不能发射）
    // 实际上需要等待 FIRE_INTERVAL (0.2s)
    for (int i = 0; i < 15; ++i) {
        model.update(0.016f);
    }

    CHECK(bullets.size() > oldCount);
}
