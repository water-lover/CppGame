#include <catch2/catch_test_macros.hpp>
#include "Model/GameModel.hpp"

TEST_CASE("GameModel - initial state", "[model]") {
    GameModel model;

    CHECK(model.getScore()  == 0);
    CHECK(model.isRunning() == false);
    CHECK(model.getTimeLeft() > 0);
    CHECK(model.getLevel() == 1);
}

TEST_CASE("GameModel - reset starts game", "[model]") {
    GameModel model;

    model.reset();

    CHECK(model.getScore()  == 0);
    CHECK(model.isRunning() == true);
    CHECK(model.getTimeLeft() > 0);
    CHECK(model.getLevel() == 1);
}

TEST_CASE("GameModel - hitTarget adds score", "[model]") {
    GameModel model;
    model.reset();

    int oldScore = model.getScore();
    bool hit = model.hitTarget(model.getTargetX(), model.getTargetY());

    CHECK(hit == true);
    CHECK(model.getScore() == oldScore + 10);
}

TEST_CASE("GameModel - miss target", "[model]") {
    GameModel model;
    model.reset();

    // 点击很远的地方
    bool hit = model.hitTarget(0.99f, 0.99f);

    CHECK(hit == false);
    CHECK(model.getScore() == 0);
}

TEST_CASE("GameModel - update counts down", "[model]") {
    GameModel model;
    model.reset();

    float t0 = model.getTimeLeft();
    model.update(1.0f);
    float t1 = model.getTimeLeft();

    CHECK(t1 < t0);
}

TEST_CASE("GameModel - time runs out stops game", "[model]") {
    GameModel model;
    model.reset();

    // 模拟 31 秒
    for (int i = 0; i < 200; i++) {
        model.update(0.5f);
    }

    CHECK(model.isRunning() == false);
    CHECK(model.getTimeLeft() == 0.0f);
}
