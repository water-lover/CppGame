#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/CollisionSystem.hpp"
#include "common/Logger.hpp"
#include <chrono>
#include <algorithm>
#include <cmath>

GameMapVM::GameMapVM(QObject* parent)
    : QObject(parent)
    , m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
{
}

// ═══════════════════════════════════════════════════════════════════
// 命令 getter
// ═══════════════════════════════════════════════════════════════════

std::function<void()> GameMapVM::getStartGameCommand() {
    return [this]() { startGameImpl(); };
}
std::function<void(int)> GameMapVM::getMoveUpCommand() {
    return [this](int active) { moveUpImpl(active); };
}
std::function<void(int)> GameMapVM::getMoveDownCommand() {
    return [this](int active) { moveDownImpl(active); };
}
std::function<void(int)> GameMapVM::getMoveLeftCommand() {
    return [this](int active) { moveLeftImpl(active); };
}
std::function<void(int)> GameMapVM::getMoveRightCommand() {
    return [this](int active) { moveRightImpl(active); };
}
std::function<void(float)> GameMapVM::getTickCommand() {
    return [this](float dt) { tickImpl(dt); };
}
std::function<void(int)> GameMapVM::getSelectModeCommand() {
    return [this](int mode) { selectModeImpl(mode); };
}
std::function<void()> GameMapVM::getPauseCommand() {
    return [this]() { pauseImpl(); };
}
std::function<void(int)> GameMapVM::getSelectAircraftCommand() {
    return [this](int type) { selectAircraftImpl(type); };
}
std::function<void()> GameMapVM::getUseSkillCommand() {
    return [this]() { useSkillImpl(); };
}

// ═══════════════════════════════════════════════════════════════════
// 命令实现
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::startGameImpl() {
    m_state = GameState::Playing;
    m_player.reset();
    m_enemies.clear();
    m_bullets.clear();
    m_scoreMgr.reset();
    m_elapsed     = 0.0f;
    m_lastScore   = 0;
    m_lastLives   = m_player.getLives();
    m_lastGameOver = false;
    m_lastSkillCD  = 0.0f;
    m_lastWeaponLv = m_player.getWeaponLevel();
    m_flameStormTimer = 0.0f;
    m_isDashing = false;
    m_dashTimer = 0.0f;

    // 初始化波次和道具管理器
    m_waveMgr.reset(m_mode == GameMode::Campaign ? m_currentLevel : 1);
    m_waveMgr.setEndless(m_mode == GameMode::Endless);
    m_powerUpMgr.reset();

    // 初始化技能
    m_skill.init(m_player.getAircraftType());

    syncMap();
    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_SCORE);
    fireChange(PROP_ID_LIVES);
    fireChange(PROP_ID_GAME_STATE);
    fireChange(PROP_ID_SKILL_COOLDOWN);
    fireChange(PROP_ID_WEAPON_LEVEL);
    fireChange(PROP_ID_WAVE);

    log("GameMapVM", "Game started with " +
        std::string(AircraftStats::getTemplate(m_player.getAircraftType()).name));
}

void GameMapVM::tickImpl(float dt) {
    if (m_state != GameState::Playing) return;

    m_elapsed += dt;

    // 1. 更新玩家
    m_player.update(dt);

    // 2. 更新技能冷却/持续
    m_skill.update(dt);
    applySkillEffects();

    // 3. 玩家自动射击
    if (m_player.canFire(dt)) {
        const Vec2 p = m_player.getPos();
        const int wl = m_player.getWeaponLevel();
        const auto& tmpl = AircraftStats::getTemplate(m_player.getAircraftType());
        const float bx = p.x, by = p.y - m_player.getSize();

        if (tmpl.type == AircraftType::Flame) {
            m_bullets.emplace_back(bx, by, 0.0f, -BULLET_SPEED, Bullet::Player);
            m_bullets.emplace_back(bx - 0.03f, by, -0.15f, -BULLET_SPEED * 0.95f, Bullet::Player);
            m_bullets.emplace_back(bx + 0.03f, by, 0.15f, -BULLET_SPEED * 0.95f, Bullet::Player);
        } else if (tmpl.type == AircraftType::Fortress) {
            m_bullets.emplace_back(bx - 0.02f, by, -0.05f, -BULLET_SPEED, Bullet::Player);
            m_bullets.emplace_back(bx + 0.02f, by, 0.05f, -BULLET_SPEED, Bullet::Player);
        } else if (tmpl.type == AircraftType::Phantom) {
            m_bullets.emplace_back(bx, by, 0.0f, -BULLET_SPEED * 1.2f, Bullet::Player);
        } else {
            m_bullets.emplace_back(bx - 0.015f, by, 0.0f, -BULLET_SPEED, Bullet::Player);
            m_bullets.emplace_back(bx + 0.015f, by, 0.0f, -BULLET_SPEED, Bullet::Player);
            if (wl >= 3) {
                m_bullets.emplace_back(bx, by, 0.0f, -BULLET_SPEED, Bullet::Player);
            }
        }
    }

    // 4. 波次管理器生成敌机
    m_waveMgr.update(dt, m_enemies, m_player.getPos().y, m_rng);

    // 5. 更新敌机
    for (auto& e : m_enemies) {
        if (!e->isDead()) e->update(dt);
    }

    // 6. 敌机攻击（发射子弹）
    handleEnemyAttacks(dt);

    // 7. 更新子弹
    for (auto& b : m_bullets) b.update(dt);

    // 8. 更新道具下落
    m_powerUpMgr.update(dt);

    // 9. 碰撞检测
    checkCollisions();

    // 10. 道具拾取
    int pickupType = m_powerUpMgr.checkPickup(m_player.getPos(), m_player.getSize());
    if (pickupType >= 0) {
        switch (static_cast<PowerUpType>(pickupType)) {
        case PowerUpType::Hp:
            // 回血（但不超过 maxLives）
            break;
        case PowerUpType::Fire:
            m_player.setWeaponLevel(m_player.getWeaponLevel() + 1);
            break;
        case PowerUpType::Shield:
            m_player.setShielded(true);
            break;
        }
        fireChange(PROP_ID_WEAPON_LEVEL);
    }

    // 11. 清理
    cleanupEntities();

    // 12. 更新波次/BOSS 状态
    m_wave = m_waveMgr.getCurrentWave();
    m_currentLevel = m_waveMgr.getCurrentLevel();
    // 查找 BOSS 血量
    m_bossHp = 0;
    m_bossMaxHp = 0;
    for (const auto& e : m_enemies) {
        auto* boss = dynamic_cast<Boss*>(e.get());
        if (boss && !boss->isDead()) {
            m_bossHp = boss->getHp();
            m_bossMaxHp = boss->getMaxHp();
            break;
        }
    }

    // 13. 关卡完成 → 进入下一关
    if (m_waveMgr.isLevelComplete(m_enemies)) {
        int nextLevel = m_currentLevel + 1;
        if (nextLevel > 7) {
            // 通关后回到模式选择或无尽
            log("GameMapVM", "All 7 levels cleared!");
            m_state = GameState::GameOver;  // 暂时以 GameOver 处理
            fireChange(PROP_ID_GAME_STATE);
            return;
        }
        m_waveMgr.reset(nextLevel);
        m_currentLevel = nextLevel;
        m_wave = 0;
        m_bossHp = 0;
        m_bossMaxHp = 0;
        log("GameMapVM", "Advancing to level " + std::to_string(nextLevel));
        fireChange(PROP_ID_WAVE);
        fireChange(PROP_ID_BOSS_HEALTH);
        fireChange(PROP_ID_GAME_STATE);
    }

    // 14. 同步到 AirMap
    syncMap();

    // 14. 发出属性变化通知
    int curScore = m_scoreMgr.getScore();
    if (curScore != m_lastScore) {
        m_lastScore = curScore;
        fireChange(PROP_ID_SCORE);
    }
    int curLives = m_player.getLives();
    if (curLives != m_lastLives) {
        m_lastLives = curLives;
        fireChange(PROP_ID_LIVES);
    }
    float curCD = m_skill.getCooldownPercent();
    if (std::abs(curCD - m_lastSkillCD) > 0.01f) {
        m_lastSkillCD = curCD;
        fireChange(PROP_ID_SKILL_COOLDOWN);
    }
    int curWL = m_player.getWeaponLevel();
    if (curWL != m_lastWeaponLv) {
        m_lastWeaponLv = curWL;
        fireChange(PROP_ID_WEAPON_LEVEL);
    }

    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_BOSS_HEALTH);
    fireChange(PROP_ID_WAVE);

    // 游戏结束
    bool isOver = (m_state == GameState::GameOver);
    if (isOver != m_lastGameOver) {
        m_lastGameOver = isOver;
        if (isOver) {
            if (m_scoreMgr.getScore() > m_scoreMgr.getHighScore()) {
                m_scoreMgr.setHighScore(m_scoreMgr.getScore());
            }
        }
        fireChange(PROP_ID_GAME_STATE);
    }
}

void GameMapVM::moveUpImpl(int active)    { m_player.moveUp(active != 0); }
void GameMapVM::moveDownImpl(int active)  { m_player.moveDown(active != 0); }
void GameMapVM::moveLeftImpl(int active)  { m_player.moveLeft(active != 0); }
void GameMapVM::moveRightImpl(int active) { m_player.moveRight(active != 0); }

void GameMapVM::selectModeImpl(int mode) {
    m_mode = (mode == 0) ? GameMode::Campaign : GameMode::Endless;
    startGameImpl();
}

void GameMapVM::pauseImpl() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
    }
    fireChange(PROP_ID_GAME_STATE);
}

void GameMapVM::selectAircraftImpl(int type) {
    if (type < 0 || type >= AircraftStats::count()) return;
    m_player.setAircraftType(static_cast<AircraftType>(type));
}

void GameMapVM::useSkillImpl() {
    if (m_state != GameState::Playing) return;
    if (!m_skill.activate()) return;

    const auto& tmpl = AircraftStats::getTemplate(m_player.getAircraftType());
    switch (tmpl.skill) {
    case SkillType::ThunderStrike: handleThunderStrike(); break;
    case SkillType::FrostShield:   m_player.setShielded(true); break;
    case SkillType::IronWall:      break; // in applySkillEffects
    case SkillType::FlameStorm:    break;
    case SkillType::TimeDash:      break;
    }
    fireChange(PROP_ID_SKILL_COOLDOWN);
}

// ═══════════════════════════════════════════════════════════════════
// 敌机攻击
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::handleEnemyAttacks(float dt) {
    float playerX = m_player.getPos().x;
    for (auto& e : m_enemies) {
        if (e->isDead()) continue;
        if (e->canAttack(dt)) {
            e->attack(m_bullets, playerX);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// 技能效果处理
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::applySkillEffects() {
    if (!m_skill.isActive()) {
        m_flameStormTimer = 0.0f;
        m_isDashing = false;
        return;
    }
    const auto& tmpl = AircraftStats::getTemplate(m_player.getAircraftType());
    const float dt = 0.016f;
    switch (tmpl.skill) {
    case SkillType::FlameStorm: handleFlameStorm(dt); break;
    case SkillType::TimeDash:   handleTimeDash(dt);   break;
    default: break;
    }
}

void GameMapVM::handleThunderStrike() {
    for (auto& e : m_enemies) {
        for (int i = 0; i < 3; ++i) {
            if (!e->isDead()) e->takeDamage();
        }
        if (e->isDead()) {
            m_powerUpMgr.onEnemyDestroyed(e->getPos(), m_rng);
        }
    }
}

void GameMapVM::handleFlameStorm(float dt) {
    m_flameStormTimer += dt;
    if (m_flameStormTimer >= 0.1f) {
        m_flameStormTimer = 0.0f;
        Vec2 p = m_player.getPos();
        for (int i = -2; i <= 2; ++i) {
            m_bullets.emplace_back(p.x + i * 0.02f, p.y - m_player.getSize(),
                                   i * 0.15f, -BULLET_SPEED * 0.9f, Bullet::Player);
        }
    }
}

void GameMapVM::handleTimeDash(float dt) {
    if (!m_isDashing) {
        m_isDashing = true;
        m_dashTimer = 0.3f;
        for (auto& e : m_enemies) {
            if (e->isDead()) continue;
            float ex = e->getPos().x, ey = e->getPos().y;
            if (ey < m_player.getPos().y && ey > m_player.getPos().y - 0.5f &&
                std::abs(ex - m_player.getPos().x) < 0.15f) {
                e->takeDamage();
                if (e->isDead()) m_powerUpMgr.onEnemyDestroyed(e->getPos(), m_rng);
            }
        }
    }
    m_dashTimer -= dt;
    if (m_dashTimer <= 0.0f) m_isDashing = false;
}

// ═══════════════════════════════════════════════════════════════════
// 碰撞检测
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::checkCollisions() {
    bool reflectEnabled = m_skill.isActive()
        && AircraftStats::getTemplate(m_player.getAircraftType()).skill == SkillType::IronWall;

    // 子弹 vs 敌机（包括 BOSS）
    for (size_t bi = 0; bi < m_bullets.size(); ++bi) {
        if (m_bullets[bi].getOwner() != Bullet::Player) continue;
        if (m_bullets[bi].isOffScreen()) continue;

        for (size_t ei = 0; ei < m_enemies.size(); ++ei) {
            if (m_enemies[ei]->isDead()) continue;
            float dx = m_bullets[bi].getPos().x - m_enemies[ei]->getPos().x;
            float dy = m_bullets[bi].getPos().y - m_enemies[ei]->getPos().y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < m_bullets[bi].getSize() + m_enemies[ei]->getSize()) {
                m_enemies[ei]->takeDamage();
                if (m_enemies[ei]->isDead()) {
                    m_scoreMgr.addScore(m_enemies[ei]->getScore());
                    m_powerUpMgr.onEnemyDestroyed(m_enemies[ei]->getPos(), m_rng);
                }
                m_bullets[bi] = Bullet(); // 标记移除
                break;
            }
        }
    }

    // 敌方子弹 vs 玩家
    for (auto& b : m_bullets) {
        if (b.getOwner() != Bullet::Enemy) continue;
        float dx = b.getPos().x - m_player.getPos().x;
        float dy = b.getPos().y - m_player.getPos().y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < m_player.getSize() + b.getSize()) {
            if (reflectEnabled) continue; // 铁壁阵反弹
            m_player.takeDamage();
            b = Bullet(); // 移除
            if (m_player.isDead()) {
                m_state = GameState::GameOver;
                log("GameMapVM", "Player died — Game Over");
                return;
            }
        }
    }

    // 敌机 vs 玩家
    for (auto& e : m_enemies) {
        if (e->isDead()) continue;
        float dx = e->getPos().x - m_player.getPos().x;
        float dy = e->getPos().y - m_player.getPos().y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < m_player.getSize() + e->getSize()) {
            m_player.takeDamage();
            e->takeDamage();
            if (m_player.isDead()) {
                m_state = GameState::GameOver;
                log("GameMapVM", "Player died — Game Over");
                return;
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// 清理
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::spawnEnemy() {
    // 已由 WaveManager 接管，此函数保留以兼容旧调用
}

void GameMapVM::cleanupEntities() {
    auto removeDead = [](auto& container, auto isDead) {
        container.erase(
            std::remove_if(container.begin(), container.end(), isDead),
            container.end());
    };
    // 敌机：死亡 或 离屏（BOSS 不退场）
    removeDead(m_enemies, [](const auto& e) {
        if (dynamic_cast<const Boss*>(e.get())) {
            return e->isDead(); // BOSS 死亡才移除
        }
        return e->isDead() || e->isOffScreen();
    });
    // 子弹：离屏 或 已被标记移除（通过 getSize() == 0 判断）
    removeDead(m_bullets, [](const auto& b) {
        return b.isOffScreen() || b.getSize() < 0.001f;
    });
}

// ═══════════════════════════════════════════════════════════════════
// syncMap — 同步到 AirMap
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::syncMap() {
    m_map.clear();

    // 玩家
    {
        Actor a;
        a.type  = ActorType::Player;
        a.x     = m_player.getPos().x;
        a.y     = m_player.getPos().y;
        a.hp    = m_player.getLives();
        a.maxHp = AircraftStats::getTemplate(m_player.getAircraftType()).baseLives;
        m_map.append(a);
    }

    // 敌机 / BOSS
    for (const auto& e : m_enemies) {
        if (!e->isDead()) {
            Actor a;
            a.type = e->getActorType();
            a.x    = e->getPos().x;
            a.y    = e->getPos().y;
            a.hp   = e->getHp();

            auto* boss = dynamic_cast<Boss*>(e.get());
            if (boss) {
                a.maxHp = boss->getMaxHp();
                a.type = ActorType::Boss;
            } else {
                a.maxHp = 1;
            }
            m_map.append(a);
        }
    }

    // 子弹
    for (const auto& b : m_bullets) {
        if (b.getSize() < 0.001f) continue; // 已标记移除
        Actor a;
        a.type  = (b.getOwner() == Bullet::Player) ? ActorType::PlayerBullet : ActorType::EnemyBullet;
        a.x     = b.getPos().x;
        a.y     = b.getPos().y;
        a.hp    = 1;
        a.maxHp = 1;
        m_map.append(a);
    }

    // 道具
    for (const auto& pu : m_powerUpMgr.getPowerUps()) {
        Actor a;
        switch (pu.type) {
        case PowerUpType::Hp:     a.type = ActorType::PowerUpHp; break;
        case PowerUpType::Fire:   a.type = ActorType::PowerUpFire; break;
        case PowerUpType::Shield: a.type = ActorType::PowerUpShield; break;
        }
        a.x = pu.pos.x;
        a.y = pu.pos.y;
        a.hp = 1; a.maxHp = 1;
        m_map.append(a);
    }
}

void GameMapVM::fireChange(uint32_t id) {
    emit propertyChanged(id);
}
