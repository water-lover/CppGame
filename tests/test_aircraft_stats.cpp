// ── AircraftStats 单元测试 ─────────────────────────────────────────────────
// 测试 5 架战机的属性模板定义是否正确
//
// 覆盖范围：
//   - 全部 5 种战机类型存在且可查询
//   - 每架战机的星级/数值对齐 DESIGN_PLAN.md 4.2 节
//   - 技能类型/冷却/持续时间正确
//   - 边界条件：非法类型、count 值

#include <catch2/catch_test_macros.hpp>
#include "viewmodel/AircraftStats.hpp"

TEST_CASE("AircraftStats - count returns 5 aircraft", "[aircraft][stats]") {
    CHECK(AircraftStats::count() == 5);
}

TEST_CASE("AircraftStats - Thunder is balanced all-rounder", "[aircraft][stats]") {
    const auto& t = AircraftStats::getTemplate(AircraftType::Thunder);

    CHECK(t.type == AircraftType::Thunder);
    CHECK(t.name == std::string("雷霆号"));
    CHECK(t.starFirePower == 3);       // ★★★
    CHECK(t.baseLives == 3);           // ♥♥♥
    CHECK(t.speedMultiplier == 1.0f);  // ★★★ (基准)
    CHECK(t.fireInterval == 0.20f);    // 基准射速
    CHECK(t.skill == SkillType::ThunderStrike);
    CHECK(t.skillCooldown == 15.0f);
    CHECK(t.skillDuration == 0.0f);    // 瞬间全屏
}

TEST_CASE("AircraftStats - Flame has highest firepower", "[aircraft][stats]") {
    const auto& t = AircraftStats::getTemplate(AircraftType::Flame);

    CHECK(t.type == AircraftType::Flame);
    CHECK(t.name == std::string("烈焰号"));
    CHECK(t.starFirePower == 5);       // ★★★★★ (最高)
    CHECK(t.baseLives == 2);           // ♥♥
    CHECK(t.speedMultiplier == 0.85f); // ★★
    CHECK(t.skill == SkillType::FlameStorm);
    CHECK(t.skillCooldown == 18.0f);
    CHECK(t.skillDuration == 2.0f);    // 持续 2s
}

TEST_CASE("AircraftStats - Frost has most lives", "[aircraft][stats]") {
    const auto& t = AircraftStats::getTemplate(AircraftType::Frost);

    CHECK(t.type == AircraftType::Frost);
    CHECK(t.name == std::string("冰霜号"));
    CHECK(t.starFirePower == 2);       // ★★
    CHECK(t.baseLives == 5);           // ♥♥♥♥♥ (最多)
    CHECK(t.speedMultiplier == 0.85f); // ★★
    CHECK(t.skill == SkillType::FrostShield);
    CHECK(t.skillCooldown == 20.0f);
    CHECK(t.skillDuration == 4.0f);    // 护盾持续 4s
}

TEST_CASE("AircraftStats - Phantom is fastest", "[aircraft][stats]") {
    const auto& t = AircraftStats::getTemplate(AircraftType::Phantom);

    CHECK(t.type == AircraftType::Phantom);
    CHECK(t.name == std::string("幻影号"));
    CHECK(t.starFirePower == 3);       // ★★★
    CHECK(t.baseLives == 2);           // ♥♥
    CHECK(t.speedMultiplier == 1.4f);  // ★★★★★ (最快)
    CHECK(t.fireInterval == 0.15f);    // 极快射速
    CHECK(t.skill == SkillType::TimeDash);
    CHECK(t.skillCooldown == 16.0f);
    CHECK(t.skillDuration == 0.3f);    // 冲刺持续 0.3s
}

TEST_CASE("AircraftStats - Fortress is tank with most defense", "[aircraft][stats]") {
    const auto& t = AircraftStats::getTemplate(AircraftType::Fortress);

    CHECK(t.type == AircraftType::Fortress);
    CHECK(t.name == std::string("堡垒号"));
    CHECK(t.starFirePower == 2);       // ★★
    CHECK(t.baseLives == 4);           // ♥♥♥♥
    CHECK(t.speedMultiplier == 0.7f);  // ★ (最慢)
    CHECK(t.fireInterval == 0.22f);
    CHECK(t.skill == SkillType::IronWall);
    CHECK(t.skillCooldown == 22.0f);   // 最长冷却
    CHECK(t.skillDuration == 3.0f);    // 无敌 3s
}

TEST_CASE("AircraftStats - all 5 types have unique names", "[aircraft][stats]") {
    // 验证 5 架战机名称各不相同
    std::string names[AircraftStats::count()];
    for (int i = 0; i < AircraftStats::count(); ++i) {
        auto type = static_cast<AircraftType>(i);
        names[i] = AircraftStats::getTemplate(type).name;
    }

    for (int i = 0; i < AircraftStats::count(); ++i) {
        for (int j = i + 1; j < AircraftStats::count(); ++j) {
            CHECK(names[i] != names[j]);
        }
    }
}

TEST_CASE("AircraftStats - all 5 skills are distinct", "[aircraft][stats]") {
    // 验证 5 架战机的技能各不相同
    SkillType skills[AircraftStats::count()];
    for (int i = 0; i < AircraftStats::count(); ++i) {
        auto type = static_cast<AircraftType>(i);
        skills[i] = AircraftStats::getTemplate(type).skill;
    }

    for (int i = 0; i < AircraftStats::count(); ++i) {
        for (int j = i + 1; j < AircraftStats::count(); ++j) {
            CHECK(skills[i] != skills[j]);
        }
    }
}

TEST_CASE("AircraftStats - each template has valid image key", "[aircraft][stats][boundary]") {
    for (int i = 0; i < AircraftStats::count(); ++i) {
        auto type = static_cast<AircraftType>(i);
        const auto& t = AircraftStats::getTemplate(type);
        // imageKey 不应为空
        CHECK(t.imageKey != nullptr);
        CHECK(std::string(t.imageKey).length() > 0);
    }
}

TEST_CASE("AircraftStats - baseLives and starFirePower in valid range", "[aircraft][stats][boundary]") {
    for (int i = 0; i < AircraftStats::count(); ++i) {
        auto type = static_cast<AircraftType>(i);
        const auto& t = AircraftStats::getTemplate(type);

        CHECK(t.baseLives >= 1);
        CHECK(t.baseLives <= 5);
        CHECK(t.starFirePower >= 1);
        CHECK(t.starFirePower <= 5);
        CHECK(t.speedMultiplier > 0.0f);
        CHECK(t.fireInterval > 0.0f);
        CHECK(t.skillCooldown > 0.0f);
        CHECK(t.skillDuration >= 0.0f);
    }
}

TEST_CASE("AircraftStats - enum values match template array order", "[aircraft][stats][boundary]") {
    // 验证 AircraftType 枚举值 0~4 对应数组 0~4
    CHECK(static_cast<int>(AircraftType::Thunder)  == 0);
    CHECK(static_cast<int>(AircraftType::Flame)    == 1);
    CHECK(static_cast<int>(AircraftType::Frost)    == 2);
    CHECK(static_cast<int>(AircraftType::Phantom)  == 3);
    CHECK(static_cast<int>(AircraftType::Fortress) == 4);
}
