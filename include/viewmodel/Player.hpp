#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "common/MathUtils.hpp"
#include "common/Constants.hpp"
#include "viewmodel/AircraftStats.hpp"

/// 玩家数据类
///
/// 迭代 3 新增：
///   - aircraftType_ / weaponLevel_ — 战机选择和武器升级
///   - hasShield_ — 护盾状态（冰霜号极寒护盾/堡垒号铁壁阵）
///   - stats-based 速度和射击参数
class Player {
public:
    Player();

    /// 重置到初始状态（使用当前 aircraftType 的模板）
    void reset();

    /// 选择战机（在 reset 前调用）
    void setAircraftType(AircraftType type) { m_aircraftType = type; }
    AircraftType getAircraftType() const { return m_aircraftType; }

    /// 每帧更新
    void update(float dt);

    // ── 移动 ──────────────────────────────────────────────────────
    void moveUp(bool active);
    void moveDown(bool active);
    void moveLeft(bool active);
    void moveRight(bool active);

    // ── 生命 ──────────────────────────────────────────────────────
    int  getLives()  const { return m_lives; }
    int  getMaxLives() const;
    void heal(int amount);
    void takeDamage();
    bool isInvincible() const { return m_invincibleTimer > 0.0f; }
    bool isDead()     const { return m_lives <= 0; }

    // ── 护盾（冰霜号技能 / 堡垒号技能 / 道具） ──────────────────
    void setShielded(bool shielded) { m_hasShield = shielded; }
    bool hasShield() const { return m_hasShield; }
    const bool* getHasShieldPtr() const { return &m_hasShield; }

    // ── 技能无敌（与道具护盾分开跟踪） ───────────────────────────
    void setSkillInvincible(bool v) { m_skillInvincible = v; }
    bool isSkillInvincible() const { return m_skillInvincible; }

    // ── 武器等级 ──────────────────────────────────────────────────
    int  getWeaponLevel()    const { return m_weaponLevel; }
    void setWeaponLevel(int level) { m_weaponLevel = (level < 1) ? 1 : (level > 5) ? 5 : level; }
    int  getMaxWeaponLevel() const { return 5; }

    // ── 射击 ──────────────────────────────────────────────────────
    bool canFire(float dt);
    void resetFireTimer();

    // ── 位置 ──────────────────────────────────────────────────────
    Vec2  getPos()    const { return m_pos; }
    float getSize()   const { return PLAYER_SIZE; }
    void  setPos(Vec2 p) { m_pos = p; }

    // ── 属性（基于战机模板） ──────────────────────────────────────
    float getFireInterval() const;
    float getSpeedValue() const;

private:
    Vec2  m_pos;
    int   m_lives        = PLAYER_MAX_LIVES;
    float m_invincibleTimer = 0.0f;
    float m_fireTimer    = 0.0f;

    bool  m_moveUp    = false;
    bool  m_moveDown  = false;
    bool  m_moveLeft  = false;
    bool  m_moveRight = false;

    // 迭代 3 新增成员
    AircraftType m_aircraftType = AircraftType::Thunder;  // 默认雷霆号
    int   m_weaponLevel  = 1;      // 武器等级 1~5
    bool  m_hasShield    = false;  // 护盾状态（来自道具或技能）
    bool  m_skillInvincible = false;  // 技能无敌（与道具护盾分开）
};

#endif // PLAYER_HPP
