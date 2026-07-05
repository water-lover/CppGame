#include <catch2/catch_test_macros.hpp>
#include "Model/GameModel.hpp"

// ── GameModel Unit Tests ───────────────────────────────────────────────────

TEST_CASE("GameModel - initial state", "[model]") {
    GameModel model;

    CHECK(model.getScore()  == 0);
    CHECK(model.isRunning() == true);
}

TEST_CASE("GameModel - reset clears state", "[model]") {
    GameModel model;

    model.addScore(100);
    model.setRunning(false);

    REQUIRE(model.getScore()  == 100);
    REQUIRE(model.isRunning() == false);

    model.reset();

    CHECK(model.getScore()  == 0);
    CHECK(model.isRunning() == true);
}

TEST_CASE("GameModel - addScore accumulates", "[model]") {
    GameModel model;

    model.addScore(10);
    CHECK(model.getScore() == 10);

    model.addScore(25);
    CHECK(model.getScore() == 35);
}

TEST_CASE("GameModel - update skip when not running", "[model]") {
    GameModel model;

    model.setRunning(false);
    model.update(1.0f);

    CHECK(model.getScore()  == 0);
    CHECK(model.isRunning() == false);
}
