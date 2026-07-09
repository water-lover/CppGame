#include <catch2/catch_test_macros.hpp>
#include "viewmodel/Player.hpp"
#include "common/Constants.hpp"

TEST_CASE("Player - initial state", "[player]") {
    Player player;

    CHECK(player.getLives() == PLAYER_MAX_LIVES);
    CHECK(player.isDead() == false);
    CHECK(player.isInvincible() == false);
}

TEST_CASE("Player - reset restores state", "[player]") {
    Player player;
    player.takeDamage();
    for (int j = 0; j < 200; ++j)
        player.update(0.016f);  // 等待无敌结束
    player.takeDamage();
    CHECK(player.getLives() == 1);

    player.reset();
    CHECK(player.getLives() == PLAYER_MAX_LIVES);
}

TEST_CASE("Player - invincible after damage", "[player]") {
    Player player;
    player.takeDamage();
    CHECK(player.isInvincible() == true);

    // 等待无敌时间过去
    for (int i = 0; i < 300; ++i) {
        player.update(0.016f);
    }
    CHECK(player.isInvincible() == false);
}

TEST_CASE("Player - dies when lives reach zero", "[player]") {
    Player player;
    for (int i = 0; i < 3; ++i) {
        // 每次受伤后需要等无敌结束
        player.takeDamage();
        for (int j = 0; j < 200; ++j)
            player.update(0.016f);
    }
    CHECK(player.isDead() == true);
}

TEST_CASE("Player - moves correctly", "[player]") {
    Player player;
    Vec2 initialPos = player.getPos();

    player.moveUp(true);
    player.update(1.0f);
    CHECK(player.getPos().y < initialPos.y);

    Vec2 afterUp = player.getPos();
    player.moveUp(false);
    player.moveDown(true);
    player.update(1.0f);
    CHECK(player.getPos().y > afterUp.y);
}

TEST_CASE("Player - bounded to screen", "[player]") {
    Player player;
    // 尝试移出左边界
    player.moveLeft(true);
    for (int i = 0; i < 100; ++i)
        player.update(0.1f);
    CHECK(player.getPos().x >= 0.0f);

    // 尝试移出右边界
    player.moveLeft(false);
    player.moveRight(true);
    for (int i = 0; i < 100; ++i)
        player.update(0.1f);
    CHECK(player.getPos().x <= 1.0f);
}
