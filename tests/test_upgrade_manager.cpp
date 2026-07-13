// ── UpgradeManager 单元测试 ──────────────────────────────────────────────
#include "viewmodel/GameConstants.hpp"
// 测试升级系统的消耗计算、星核管理、属性加成、序列化
//
// 覆盖范围：
//   - 初始状态（星核=0，所有等级=0）
//   - 星核加减操作
//   - 升级成功/失败条件
//   - 属性加成计算
//   - 序列化打包/解包
//   - 边界条件：max level、星核不足

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "viewmodel/UpgradeManager.hpp"

TEST_CASE("UpgradeManager - initial state", "[upgrade][init]") {
    UpgradeManager um;

    CHECK(um.getStarCores() == 0);
    CHECK(um.getStarCoresPtr() != nullptr);
    CHECK(*um.getStarCoresPtr() == 0);
    CHECK(um.getUpgradeLevel(UpgradeType::FirePower)  == 0);
    CHECK(um.getUpgradeLevel(UpgradeType::Lives)      == 0);
    CHECK(um.getUpgradeLevel(UpgradeType::Speed)      == 0);
    CHECK(um.getUpgradeLevel(UpgradeType::Cooldown)   == 0);
    CHECK(um.getFirePowerBonus()  == Approx(0.0f));
    CHECK(um.getLivesBonus()      == 0);
    CHECK(um.getSpeedBonus()      == Approx(0.0f));
    CHECK(um.getCooldownBonus()   == Approx(0.0f));
}

TEST_CASE("UpgradeManager - addStarCores accumulates correctly", "[upgrade][cores]") {
    UpgradeManager um;

    um.addStarCores(10);
    CHECK(um.getStarCores() == 10);

    um.addStarCores(25);
    CHECK(um.getStarCores() == 35);
}

TEST_CASE("UpgradeManager - spendStarCores succeeds with enough cores", "[upgrade][cores]") {
    UpgradeManager um;
    um.addStarCores(50);

    bool result = um.spendStarCores(30);
    CHECK(result == true);
    CHECK(um.getStarCores() == 20);
}

TEST_CASE("UpgradeManager - spendStarCores fails without enough cores", "[upgrade][cores][boundary]") {
    UpgradeManager um;
    um.addStarCores(10);

    bool result = um.spendStarCores(20);
    CHECK(result == false);
    CHECK(um.getStarCores() == 10);  // 余额不变
}

TEST_CASE("UpgradeManager - spendStarCores exact amount works", "[upgrade][cores][boundary]") {
    UpgradeManager um;
    um.addStarCores(30);

    CHECK(um.spendStarCores(30) == true);
    CHECK(um.getStarCores() == 0);
}

TEST_CASE("UpgradeManager - upgrade FirePower consumes cores and levels up", "[upgrade][upgrade]") {
    UpgradeManager um;
    um.addStarCores(100);

    // Lv.0 → Lv.1: 花费 10 星核
    CHECK(um.upgrade(UpgradeType::FirePower) == true);
    CHECK(um.getUpgradeLevel(UpgradeType::FirePower) == 1);
    CHECK(um.getStarCores() == 90);

    // Lv.1 → Lv.2: 花费 20 星核
    CHECK(um.upgrade(UpgradeType::FirePower) == true);
    CHECK(um.getUpgradeLevel(UpgradeType::FirePower) == 2);
    CHECK(um.getStarCores() == 70);
}

TEST_CASE("UpgradeManager - upgrade fails when not enough cores", "[upgrade][upgrade][boundary]") {
    UpgradeManager um;
    um.addStarCores(5);  // Lv.0→Lv.1 需要 10 个

    CHECK(um.upgrade(UpgradeType::FirePower) == false);
    CHECK(um.getUpgradeLevel(UpgradeType::FirePower) == 0);
    CHECK(um.getStarCores() == 5);  // 没消耗
}

TEST_CASE("UpgradeManager - upgrade fails at MAX_LEVEL", "[upgrade][upgrade][boundary]") {
    UpgradeManager um;
    um.addStarCores(9999);

    // 升到 MAX_LEVEL
    for (int i = 0; i < MAX_UPGRADE_LEVEL; ++i) {
        CHECK(um.upgrade(UpgradeType::Speed) == true);
    }
    CHECK(um.getUpgradeLevel(UpgradeType::Speed) == MAX_UPGRADE_LEVEL);

    // 满级后再升级失败
    CHECK(um.upgrade(UpgradeType::Speed) == false);
}

TEST_CASE("UpgradeManager - MAX_LEVEL constant equals 10", "[upgrade][constant]") {
    CHECK(MAX_UPGRADE_LEVEL == 10);
}

TEST_CASE("UpgradeManager - getFirePowerBonus increases with level", "[upgrade][bonus]") {
    UpgradeManager um;
    um.addStarCores(9999);

    // 每级 UPGRADE_FIRE_DELTA = 0.5
    um.upgrade(UpgradeType::FirePower);
    CHECK(um.getFirePowerBonus() == Approx(0.5f));

    um.upgrade(UpgradeType::FirePower);
    CHECK(um.getFirePowerBonus() == Approx(1.0f));
}

TEST_CASE("UpgradeManager - getLivesBonus increases with level", "[upgrade][bonus]") {
    UpgradeManager um;
    um.addStarCores(9999);

    // 每级 UPGRADE_LIVES_DELTA = 1
    um.upgrade(UpgradeType::Lives);
    CHECK(um.getLivesBonus() == 1);

    um.upgrade(UpgradeType::Lives);
    CHECK(um.getLivesBonus() == 2);
}

TEST_CASE("UpgradeManager - getSpeedBonus increases with level", "[upgrade][bonus]") {
    UpgradeManager um;
    um.addStarCores(9999);

    // 每级 UPGRADE_SPEED_DELTA = 0.05
    um.upgrade(UpgradeType::Speed);
    CHECK(um.getSpeedBonus() == Approx(0.05f));
}

TEST_CASE("UpgradeManager - getCooldownBonus caps at 50%", "[upgrade][bonus][boundary]") {
    UpgradeManager um;
    um.addStarCores(9999);

    // 升 10 级: 10 * 0.05 = 0.5 = 50% (已达到上限)
    for (int i = 0; i < MAX_UPGRADE_LEVEL; ++i)
        um.upgrade(UpgradeType::Cooldown);

    CHECK(um.getCooldownBonus() == Approx(0.5f));
}

TEST_CASE("UpgradeManager - packLevels encodes all 4 levels", "[upgrade][serialize]") {
    UpgradeManager um;

    // 设置级别
    um.setUpgradeLevel(UpgradeType::FirePower, 3);
    um.setUpgradeLevel(UpgradeType::Lives,     5);
    um.setUpgradeLevel(UpgradeType::Speed,     7);
    um.setUpgradeLevel(UpgradeType::Cooldown,  2);

    int packed = um.getAircraftLevelsPacked(0);

    // bit 0-3: FirePower=3, bit 4-7: Lives=5, bit 8-11: Speed=7, bit 12-15: Cooldown=2
    CHECK((packed & 0xF) == 3);
    CHECK(((packed >> 4) & 0xF) == 5);
    CHECK(((packed >> 8) & 0xF) == 7);
    CHECK(((packed >> 12) & 0xF) == 2);
}

TEST_CASE("UpgradeManager - unpackLevels restores levels correctly", "[upgrade][serialize]") {
    UpgradeManager um;

    // 打包数据
    int packed = (3 & 0xF) | ((5 & 0xF) << 4) | ((7 & 0xF) << 8) | ((2 & 0xF) << 12);

    um.setAircraftLevelsPacked(0, packed);

    CHECK(um.getUpgradeLevel(UpgradeType::FirePower)  == 3);
    CHECK(um.getUpgradeLevel(UpgradeType::Lives)      == 5);
    CHECK(um.getUpgradeLevel(UpgradeType::Speed)      == 7);
    CHECK(um.getUpgradeLevel(UpgradeType::Cooldown)   == 2);
}

TEST_CASE("UpgradeManager - pack/unpack roundtrip preserves data", "[upgrade][serialize]") {
    UpgradeManager um;

    um.setUpgradeLevel(UpgradeType::FirePower,  9);
    um.setUpgradeLevel(UpgradeType::Lives,      8);
    um.setUpgradeLevel(UpgradeType::Speed,      2);
    um.setUpgradeLevel(UpgradeType::Cooldown,   6);

    int packed = um.getAircraftLevelsPacked(0);

    // 新管理器解包
    UpgradeManager um2;
    um2.setAircraftLevelsPacked(0, packed);

    CHECK(um2.getUpgradeLevel(UpgradeType::FirePower)  == 9);
    CHECK(um2.getUpgradeLevel(UpgradeType::Lives)      == 8);
    CHECK(um2.getUpgradeLevel(UpgradeType::Speed)      == 2);
    CHECK(um2.getUpgradeLevel(UpgradeType::Cooldown)   == 6);
}

TEST_CASE("UpgradeManager - all 4 upgrade types work independently", "[upgrade][all]") {
    UpgradeManager um;
    um.addStarCores(9999);

    // 全部升到不同级别
    for (int i = 0; i < 3; ++i) um.upgrade(UpgradeType::FirePower);
    for (int i = 0; i < 5; ++i) um.upgrade(UpgradeType::Lives);
    for (int i = 0; i < 2; ++i) um.upgrade(UpgradeType::Speed);
    for (int i = 0; i < 1; ++i) um.upgrade(UpgradeType::Cooldown);

    CHECK(um.getUpgradeLevel(UpgradeType::FirePower)  == 3);
    CHECK(um.getUpgradeLevel(UpgradeType::Lives)      == 5);
    CHECK(um.getUpgradeLevel(UpgradeType::Speed)      == 2);
    CHECK(um.getUpgradeLevel(UpgradeType::Cooldown)   == 1);

    CHECK(um.getFirePowerBonus() == Approx(1.5f));
    CHECK(um.getLivesBonus()     == 5);
    CHECK(um.getSpeedBonus()     == Approx(0.1f));
    CHECK(um.getCooldownBonus()  == Approx(0.05f));
}

TEST_CASE("UpgradeManager - setUpgradeLevel clamps to valid range", "[upgrade][set][boundary]") {
    UpgradeManager um;

    um.setUpgradeLevel(UpgradeType::FirePower, -5);
    CHECK(um.getUpgradeLevel(UpgradeType::FirePower) == 0);

    um.setUpgradeLevel(UpgradeType::Lives, 99);
    CHECK(um.getUpgradeLevel(UpgradeType::Lives) == MAX_UPGRADE_LEVEL);
}
