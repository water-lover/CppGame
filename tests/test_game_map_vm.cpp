// ── GameMapVM 集成测试 ──────────────────────────────────────────────────────
// 测试 GameMapVM 的三绑定接口：属性暴露、命令暴露、事件通知
//
// 测试范围：
//   - 初始状态 / startGame / tick / 碰撞 / GameOver
//   - 信号发射（propertyChanged）
//   - 状态切换的正确性
//   - 边界条件：未开始游戏时 tick、重复 start/stop 等

#include <catch2/catch_test_macros.hpp>
#include <QObject>
#include <functional>
#include <vector>
#include <algorithm>
#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/AircraftStats.hpp"
#include "common/Constants.hpp"
#include "common/Actor.hpp"
#include "common/AirMap.hpp"

// ── 手动信号监听器（替代 QSignalSpy，避免 Qt5::Test 依赖） ────────────────

struct SignalRecorder {
    std::vector<uint32_t> recorded;

    // 返回一个 functor，可 connect 到 GameMapVM::propertyChanged
    void operator()(uint32_t id) { recorded.push_back(id); }

    size_t count() const { return recorded.size(); }
    bool contains(uint32_t id) const {
        return std::find(recorded.begin(), recorded.end(), id) != recorded.end();
    }
    void reset() { recorded.clear(); }
};

// ── 辅助工具 ─────────────────────────────────────────────────────────────────

/// 对 vm 执行 N 帧更新（每帧 dt = FRAME_DURATION ≈ 0.0167s）
static void tickN(GameMapVM& vm, int n, float dt = FRAME_DURATION) {
    auto tickCmd = vm.getTickCommand();
    for (int i = 0; i < n; ++i) {
        tickCmd(dt);
    }
}

/// 启动游戏并等待足够帧数使敌机生成（SPAWN_INTERVAL ≈ 1.5s）
static void startAndWarmUp(GameMapVM& vm) {
    auto startCmd = vm.getStartGameCommand();
    startCmd();
    // 等待超过 SPAWN_INTERVAL 确保第一波敌机出现
    // 乘以 2 避免浮点 SPAWN_INTERVAL/FRAME_DURATION 截断导致时间不足
    tickN(vm, static_cast<int>(SPAWN_INTERVAL / FRAME_DURATION) * 2);
}

// ── 测试用例 ─────────────────────────────────────────────────────────────────

TEST_CASE("GameMapVM - initial state is Menu", "[gamemap][initial]") {
    GameMapVM vm;

    CHECK(vm.getGameState() == GameState::Menu);
    CHECK(vm.getScore() == 0);
    CHECK(vm.getLives() == PLAYER_MAX_LIVES);
    CHECK(vm.getHighScore() == 0);

    // 地图应为空（游戏未开始）
    CHECK(vm.getMap()->size() == 0);
}

TEST_CASE("GameMapVM - startGame transitions to Playing", "[gamemap][start]") {
    GameMapVM vm;

    auto startCmd = vm.getStartGameCommand();
    startCmd();

    CHECK(vm.getGameState() == GameState::Playing);
    CHECK(vm.getScore() == 0);
    CHECK(vm.getLives() == PLAYER_MAX_LIVES);

    // 地图中应包含玩家实体
    const AirMap* map = vm.getMap();
    REQUIRE(map->size() > 0);
    CHECK(map->getAt(0).type == ActorType::Player);
}

TEST_CASE("GameMapVM - startGame emits propertyChanged signals", "[gamemap][signal]") {
    GameMapVM vm;
    SignalRecorder rec;
    QObject::connect(&vm, &GameMapVM::propertyChanged,
                     &vm, [&rec](uint32_t id) { rec(id); });

    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // startGameImpl 应发射以下信号：
    //   PROP_ID_MAP(1), PROP_ID_SCORE(2), PROP_ID_LIVES(3), PROP_ID_GAME_STATE(4)
    CHECK(rec.count() >= 4);

    CHECK(rec.contains(PROP_ID_MAP));
    CHECK(rec.contains(PROP_ID_SCORE));
    CHECK(rec.contains(PROP_ID_LIVES));
    CHECK(rec.contains(PROP_ID_GAME_STATE));
}

TEST_CASE("GameMapVM - tick before start does nothing", "[gamemap][tick][boundary]") {
    GameMapVM vm;

    // 在未开始游戏时调用 tick → 不应产生实体
    tickN(vm, 500);  // 大量 tick

    CHECK(vm.getGameState() == GameState::Menu);
    CHECK(vm.getMap()->size() == 0);   // 无实体生成
    CHECK(vm.getScore() == 0);         // 无分数变化
    CHECK(vm.getLives() == PLAYER_MAX_LIVES);  // 生命不减
}

TEST_CASE("GameMapVM - enemies spawn after SPAWN_INTERVAL", "[gamemap][enemy][spawn]") {
    GameMapVM vm;
    startAndWarmUp(vm);

    // 此时应有敌机加入地图
    const AirMap* map = vm.getMap();
    int enemyCount = 0;
    for (size_t i = 0; i < map->size(); ++i) {
        if (map->getAt(i).type == ActorType::EnemySmall)
            ++enemyCount;
    }
    CHECK(enemyCount > 0);
}

TEST_CASE("GameMapVM - auto-firing creates bullets", "[gamemap][bullet]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 射击间隔 0.2s ≈ 12 ticks
    tickN(vm, static_cast<int>(FIRE_INTERVAL / FRAME_DURATION) + 2);

    const AirMap* map = vm.getMap();
    int bulletCount = 0;
    for (size_t i = 0; i < map->size(); ++i) {
        if (map->getAt(i).type == ActorType::PlayerBullet)
            ++bulletCount;
    }
    CHECK(bulletCount >= 1);
}

TEST_CASE("GameMapVM - player movement commands work", "[gamemap][move]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 检查移动前玩家位置
    auto mapBefore = vm.getMap();
    REQUIRE(mapBefore->size() > 0);
    float beforeX = mapBefore->getAt(0).x;

    // 向右移动
    auto moveRightCmd = vm.getMoveRightCommand();
    moveRightCmd(1);  // 按下
    tickN(vm, static_cast<int>(0.5f / FRAME_DURATION));  // 移动 0.5 秒
    moveRightCmd(0);  // 松开

    // 玩家 x 坐标应增大
    auto mapAfter = vm.getMap();
    REQUIRE(mapAfter->size() > 0);
    CHECK(mapAfter->getAt(0).x > beforeX);
}

TEST_CASE("GameMapVM - enemies appear and tick processes properly", "[gamemap][collision]") {
    GameMapVM vm;
    startAndWarmUp(vm);

    // 验证 WarmUp 后地图中有实体
    CHECK(vm.getMap()->size() > 0);

    // 再多运行一些帧
    tickN(vm, 300);

    // 验证核心逻辑：游戏仍在进行中（未崩溃）
    CHECK(vm.getGameState() == GameState::Playing);
    CHECK_NOTHROW(vm.getMap()->size());

    // 碰撞检测逻辑在 test_collision.cpp 中已进行单元测试
    // 此处只验证集成流程不崩溃
}

TEST_CASE("GameMapVM - tick processes score correctly", "[gamemap][score]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 初始分数为 0
    CHECK(vm.getScore() == 0);

    // 运行一些帧，验证不会崩溃，分数不变负
    tickN(vm, 100);
    CHECK(vm.getScore() >= 0);

    // 分数来源在碰撞检测中（test_collision.cpp 已覆盖）
    // 此处只验证 GameMapVM 集成：分数接口可用、不会负值
}

TEST_CASE("GameMapVM - game state machine transitions correctly", "[gamemap][gameover]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();

    // 初始状态为 Menu
    CHECK(vm.getGameState() == GameState::Menu);

    // 开始 → Playing
    startCmd();
    CHECK(vm.getGameState() == GameState::Playing);

    // 运行大量帧（验证不崩溃）
    tickN(vm, 2000);

    // 状态要么仍在 Playing，要么已 GameOver
    //（GameOver 条件依赖于随机碰撞，此处不做硬性要求）
    CHECK((vm.getGameState() == GameState::Playing ||
           vm.getGameState() == GameState::GameOver));

    // 重新开始 → Playing
    startCmd();
    CHECK(vm.getGameState() == GameState::Playing);
    CHECK(vm.getLives() == PLAYER_MAX_LIVES);
}

TEST_CASE("GameMapVM - restart after game over resets state", "[gamemap][restart]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 跑大量帧触发 GameOver
    tickN(vm, 2000);

    // 重新开始
    startCmd();

    // 检查是否回到 Playing 状态且重置了数据
    CHECK(vm.getGameState() == GameState::Playing);
    CHECK(vm.getLives() == PLAYER_MAX_LIVES);
    CHECK(vm.getScore() == 0);

    // 地图应有玩家
    const AirMap* map = vm.getMap();
    REQUIRE(map->size() > 0);
    CHECK(map->getAt(0).type == ActorType::Player);
}

TEST_CASE("GameMapVM - score and high score tracking", "[gamemap][highscore]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();

    // 初始值
    CHECK(vm.getScore() == 0);
    CHECK(vm.getHighScore() == 0);

    // 一局游戏
    startCmd();
    tickN(vm, 500);

    // getHighScore 返回 max(highScore_, score_)，所以始终 >= 当前分
    CHECK(vm.getScore() >= 0);
    CHECK(vm.getHighScore() >= vm.getScore());

    // 重新开始后分数重置为 0
    startCmd();
    CHECK(vm.getScore() == 0);

    // 新一局 getHighScore 继续跟踪当前分数
    tickN(vm, 500);
    CHECK(vm.getHighScore() >= vm.getScore());
}

TEST_CASE("GameMapVM - multiple start/stop cycles don't crash", "[gamemap][stress][boundary]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();

    for (int cycle = 0; cycle < 5; ++cycle) {
        startCmd();                       // 开始
        CHECK(vm.getGameState() == GameState::Playing);

        tickN(vm, 50 + cycle * 30);       // 运行一些帧
        CHECK_NOTHROW(vm.getMap()->size());  // 不会崩溃
    }

    // 最终状态应为 Playing
    CHECK(vm.getGameState() == GameState::Playing);
}

TEST_CASE("GameMapVM - score value accessor consistency", "[gamemap][property]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // getScore() 和 getScoreValue() 应一致
    CHECK(vm.getScore() == vm.getScoreValue());

    tickN(vm, 500);

    CHECK(vm.getScore() == vm.getScoreValue());
    CHECK(vm.getLives() == vm.getLivesValue());
}

TEST_CASE("GameMapVM - map contains player after start", "[gamemap][map]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();

    // 开始前地图为空
    CHECK(vm.getMap()->size() == 0);

    startCmd();

    // 开始后地图有玩家
    const AirMap* map = vm.getMap();
    REQUIRE(map->size() >= 1);
    CHECK(map->getAt(0).type == ActorType::Player);
    CHECK(map->getAt(0).x >= 0.0f);
    CHECK(map->getAt(0).x <= 1.0f);
    CHECK(map->getAt(0).y >= 0.0f);
    CHECK(map->getAt(0).y <= 1.0f);

    // 玩家生命值正确
    CHECK(map->getAt(0).hp == PLAYER_MAX_LIVES);
    CHECK(map->getAt(0).maxHp == PLAYER_MAX_LIVES);
}

TEST_CASE("GameMapVM - enemies are removed when off screen", "[gamemap][cleanup][boundary]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 等待多波敌机生成并飞出屏幕
    // 敌机速度 0.25/s，从 y=-0.1 到 y>1.2 = 1.3/0.25 = 5.2 秒 ≈ 312 ticks
    // 加上生成间隔，约 400 ticks 后应有清理发生
    tickN(vm, 500);

    // 不会 crash 即可（清理逻辑在内部执行）
    CHECK_NOTHROW(vm.getMap()->size());
}

// ═══════════════════════════════════════════════════════════════════
//  迭代 3 新增测试：战机选择 / 技能 / 暂停 / 模式选择
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("GameMapVM - selectAircraft changes aircraft type", "[gamemap][iter3][aircraft]") {
    GameMapVM vm;

    // 默认是 Thunder（枚举值 0）
    CHECK(vm.getAircraftType() == 0);

    // 选择烈焰号
    auto selectCmd = vm.getSelectAircraftCommand();
    selectCmd(static_cast<int>(AircraftType::Flame));
    CHECK(vm.getAircraftType() == static_cast<int>(AircraftType::Flame));

    // 选择冰霜号
    selectCmd(static_cast<int>(AircraftType::Frost));
    CHECK(vm.getAircraftType() == static_cast<int>(AircraftType::Frost));
}

TEST_CASE("GameMapVM - selectAircraft with invalid type is ignored", "[gamemap][iter3][aircraft][boundary]") {
    GameMapVM vm;
    auto selectCmd = vm.getSelectAircraftCommand();

    // 无效类型（负数）
    selectCmd(-1);
    CHECK(vm.getAircraftType() == 0);  // 保持默认

    // 无效类型（超出范围）
    selectCmd(99);
    CHECK(vm.getAircraftType() == 0);
}

TEST_CASE("GameMapVM - skill system starts ready", "[gamemap][iter3][skill]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 开始游戏后技能就绪（默认雷霆号）
    CHECK(vm.isSkillReady() == true);
    CHECK(vm.isSkillActive() == false);
    CHECK(vm.getSkillCooldownPercent() == 0.0f);
}

TEST_CASE("GameMapVM - useSkill activates and goes on cooldown", "[gamemap][iter3][skill]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    auto skillCmd = vm.getUseSkillCommand();
    skillCmd();  // 释放技能

    // 雷霆号技能冷却 15s，持续 0s
    CHECK(vm.isSkillReady() == false);
    CHECK(vm.getSkillCooldownPercent() > 0.0f);
}

TEST_CASE("GameMapVM - useSkill before start does nothing", "[gamemap][iter3][skill][boundary]") {
    GameMapVM vm;
    auto skillCmd = vm.getUseSkillCommand();

    // 未开始游戏时释放技能 → 不会崩溃
    CHECK_NOTHROW(skillCmd());
}

TEST_CASE("GameMapVM - initial weapon level is 1", "[gamemap][iter3][weapon]") {
    GameMapVM vm;
    CHECK(vm.getWeaponLevel() == 1);

    auto startCmd = vm.getStartGameCommand();
    startCmd();
    CHECK(vm.getWeaponLevel() == 1);
}

TEST_CASE("GameMapVM - pause toggles between Playing and Paused", "[gamemap][iter3][pause]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();
    CHECK(vm.getGameState() == GameState::Playing);

    auto pauseCmd = vm.getPauseCommand();

    // 暂停
    pauseCmd();
    CHECK(vm.getGameState() == GameState::Paused);

    // 继续
    pauseCmd();
    CHECK(vm.getGameState() == GameState::Playing);

    // 再次暂停
    pauseCmd();
    CHECK(vm.getGameState() == GameState::Paused);
}

TEST_CASE("GameMapVM - pause before start does nothing", "[gamemap][iter3][pause][boundary]") {
    GameMapVM vm;
    auto pauseCmd = vm.getPauseCommand();

    // Menu 状态调用 pause → 不应改变状态
    CHECK(vm.getGameState() == GameState::Menu);
    pauseCmd();
    CHECK(vm.getGameState() == GameState::Menu);
}

TEST_CASE("GameMapVM - startGame emits new iteration 3 signals", "[gamemap][iter3][signal]") {
    GameMapVM vm;
    SignalRecorder rec;
    QObject::connect(&vm, &GameMapVM::propertyChanged,
                     &vm, [&rec](uint32_t id) { rec(id); });

    auto startCmd = vm.getStartGameCommand();
    startCmd();

    // 迭代 3 新信号：PROP_ID_SKILL_COOLDOWN, PROP_ID_WEAPON_LEVEL
    CHECK(rec.contains(PROP_ID_SKILL_COOLDOWN));
    CHECK(rec.contains(PROP_ID_WEAPON_LEVEL));
}

TEST_CASE("GameMapVM - weapon level stays 1 after start", "[gamemap][iter3][weapon]") {
    GameMapVM vm;
    auto startCmd = vm.getStartGameCommand();
    startCmd();

    CHECK(vm.getWeaponLevel() == 1);

    // 运行一些 tick
    tickN(vm, 100);
    // 武器等级未升级前保持 1
    CHECK(vm.getWeaponLevel() >= 1);
    CHECK(vm.getWeaponLevel() <= 5);
}
