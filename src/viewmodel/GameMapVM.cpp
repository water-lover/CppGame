#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/CollisionSystem.hpp"
#include "resource/Logger.hpp"
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
std::function<void(int)> GameMapVM::getStartLevelCommand() {
    return [this](int levelId) { startLevelImpl(levelId); };
}
std::function<void(int)> GameMapVM::getSelectLevelCommand() {
    return [this](int levelId) { selectLevelImpl(levelId); };
}
std::function<void()> GameMapVM::getQuitLevelCommand() {
    return [this]() { quitLevelImpl(); };
}
std::function<void(int)> GameMapVM::getSelectAircraftCommand() {
    return [this](int type) { selectAircraftImpl(type); };
}
std::function<void()> GameMapVM::getUseSkillCommand() {
    return [this]() { useSkillImpl(); };
}
std::function<void(int)> GameMapVM::getNavigateCommand() {
    return [this](int state) { navigateImpl(state); };
}
std::function<void(int)> GameMapVM::getUpgradeStatCommand() {
    return [this](int type) { upgradeStatImpl(type); };
}
std::function<void()> GameMapVM::getResetAllCommand() {
    return [this]() { resetAllImpl(); };
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
    m_lastSkillReady  = true;
    m_lastSkillActive = false;
    m_lastSkillType   = static_cast<int>(m_skill.getType());
    m_lastWeaponLv = m_player.getWeaponLevel();
    int hsSlot = (m_mode == GameMode::Endless) ? 7 : (m_currentLevel - 1);
    m_scoreMgr.setActiveHighScoreSlot(hsSlot);
    m_flameStormTimer = 0.0f;
    m_isDashing = false;
    m_dashTimer = 0.0f;
    m_levelCleared = false;

    // 初始化波次和道具管理器
    m_waveMgr.reset(m_mode == GameMode::Campaign ? m_currentLevel : 1);
    m_waveMgr.setEndless(m_mode == GameMode::Endless);
    m_powerUpMgr.reset();

    // 初始化技能
    m_skill.init(m_player.getAircraftType());

    // 应用升级加成
    m_player.setUpgradeBonuses(
        m_upgradeMgr.getFirePowerBonus(),
        m_upgradeMgr.getLivesBonus(),
        m_upgradeMgr.getSpeedBonus(),
        m_upgradeMgr.getCooldownBonus()
    );

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

    // 清空上一帧的爆炸位置队列
    m_explosionQueue.clear();

    m_elapsed += dt;

    // 1. 更新玩家
    m_player.update(dt);

    // 2. 更新技能冷却/持续
    m_skill.update(dt);
    applySkillEffects(dt);

    // 3. 玩家自动射击
    if (m_player.canFire(dt)) {
        m_scoreMgr.onShotFired();  // 迭代7：统计
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
            m_player.heal(1);
            fireChange(PROP_ID_LIVES);
            break;
        case PowerUpType::Fire:
            m_player.setWeaponLevel(m_player.getWeaponLevel() + 1);
            break;
        case PowerUpType::Shield:
            m_player.setShielded(true);
            break;
        case PowerUpType::StarCore:
            m_upgradeMgr.addStarCores(3);  // 拾取星核+3
            fireChange(PROP_ID_STAR_CORES);
            break;
        }
        fireChange(PROP_ID_WEAPON_LEVEL);
    }

    // 10. 碰撞后：检查 BOSS 是否死亡（必须在 cleanup 之前）
    for (const auto& e : m_enemies) {
        auto* boss = dynamic_cast<Boss*>(e.get());
        if (boss && boss->isDead()) {
            m_waveMgr.notifyBossDefeated();
            m_scoreMgr.addScore(boss->getScore());  // BOSS 击杀分数
            m_scoreMgr.onBossKilled();
            m_upgradeMgr.addStarCores(STAR_CORE_PER_BOSS);
            fireChange(PROP_ID_SCORE);
            fireChange(PROP_ID_STAR_CORES);
            // BOSS 死亡爆炸
            m_explosionQueue.push_back(boss->getPos());
            break;
        }
    }

    // 11. 清理实体
    cleanupEntities();

    // 12. 更新波次/BOSS 状态
    m_wave = m_waveMgr.getCurrentWave();
    m_scoreMgr.setWaveReached(m_wave);
    m_scoreMgr.addTime(dt);
    m_currentLevel = m_waveMgr.getCurrentLevel();
    // 格式化波次显示文字
    {
        if (m_mode == GameMode::Endless) {
            m_waveDisplayBuf = std::to_string(m_waveMgr.getEndlessLoop())
                + "-" + std::to_string(m_currentLevel);
        } else {
            m_waveDisplayBuf = "WAVE " + std::to_string(m_wave);
        }
    }
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

    // 13. 闯关完成 → 标记胜利，退出到选关
    if (m_mode == GameMode::Campaign && m_waveMgr.isLevelComplete(m_enemies)) {
        int nextLevel = m_currentLevel + 1;
        m_levelCleared = true;
        if (nextLevel > 7) {
            log("GameMapVM", "🎉 All 7 levels cleared! Victory!");
        } else {
            log("GameMapVM", "Level " + std::to_string(m_currentLevel) + " cleared!");
        }
        int hsSlot2 = m_currentLevel - 1;
        if (m_scoreMgr.getScore() > m_scoreMgr.getHighScore(hsSlot2)) {
            m_scoreMgr.setHighScore(hsSlot2, m_scoreMgr.getScore());
            m_scoreMgr.setActiveHighScoreSlot(hsSlot2);
            emit saveHighScoreRequested(m_scoreMgr.getScore(), hsSlot2);
        }
        m_lastScore = m_scoreMgr.getScore();

        // 更新最大已解锁关卡（ViewModel 自身业务逻辑，不依赖 App）
        if (nextLevel <= 7 && nextLevel > m_maxUnlockedLevel) {
            m_maxUnlockedLevel = nextLevel;
            fireChange(PROP_ID_MAX_UNLOCKED_LEVEL);
        }

        if (nextLevel <= 7) {
            emit saveCampaignRequested(nextLevel);
        }
        m_state = GameState::LevelComplete;  // 使用专用胜利状态
        fireChange(PROP_ID_GAME_STATE);
        return;
    }

    // 14. 同步到 AirMap
    syncMap();

    // 15. 发出属性变化通知
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
    {
        bool ready = !m_skill.isOnCooldown();
        if (ready != m_lastSkillReady) {
            m_lastSkillReady = ready;
            fireChange(PROP_ID_SKILL_COOLDOWN);
        }
    }
    {
        bool active = m_skill.isActive();
        if (active != m_lastSkillActive) {
            m_lastSkillActive = active;
            m_lastSkillType = static_cast<int>(m_skill.getType());
            fireChange(PROP_ID_SKILL_COOLDOWN);
        }
    }
    int curWL = m_player.getWeaponLevel();
    if (curWL != m_lastWeaponLv) {
        m_lastWeaponLv = curWL;
        fireChange(PROP_ID_WEAPON_LEVEL);
    }

    // 雷击特效计时
    if (m_thunderActive) {
        m_thunderTimer -= dt;
        if (m_thunderTimer <= 0.0f) m_thunderActive = false;
    }

    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_BOSS_HEALTH);
    fireChange(PROP_ID_WAVE);

    // 游戏结束
    bool isOver = (m_state == GameState::GameOver);
    if (isOver != m_lastGameOver) {
        m_lastGameOver = isOver;
        if (isOver) {
            int goSlot = (m_mode == GameMode::Endless) ? 7 : (m_currentLevel - 1);
            if (m_scoreMgr.getScore() > m_scoreMgr.getHighScore(goSlot)) {
                m_scoreMgr.setHighScore(goSlot, m_scoreMgr.getScore());
                m_scoreMgr.setActiveHighScoreSlot(goSlot);
                emit saveHighScoreRequested(m_scoreMgr.getScore(), goSlot);
            }
            m_lastScore = m_scoreMgr.getScore();
            // 游戏结束时持久化本局获得的星核
            int pp[5]; m_upgradeMgr.packAllLevels(pp);
            emit saveUpgradeRequested(m_upgradeMgr.getStarCores(),
                                      pp[0], pp[1], pp[2], pp[3], pp[4]);
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
    if (m_mode == GameMode::Campaign) {
        m_state = GameState::LevelSelect;
        fireChange(PROP_ID_GAME_STATE);
    }
    // 无尽模式：不开始游戏，等 AircraftSelect 确认后再 startGameImpl
}

void GameMapVM::startLevelImpl(int levelId) {
    if (levelId < 1 || levelId > 7) return;
    m_currentLevel = levelId;
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
    m_lastSkillReady  = true;
    m_lastSkillActive = false;
    m_lastSkillType   = static_cast<int>(m_skill.getType());
    m_lastWeaponLv = m_player.getWeaponLevel();
    int hsSlot = (m_mode == GameMode::Endless) ? 7 : (m_currentLevel - 1);
    m_scoreMgr.setActiveHighScoreSlot(hsSlot);
    m_flameStormTimer = 0.0f;
    m_isDashing = false;
    m_dashTimer = 0.0f;
    m_levelCleared = false;

    // ⚠️ 用选定的关卡 ID 初始化波次管理器
    m_waveMgr.reset(m_currentLevel);
    m_waveMgr.setEndless(false);
    m_powerUpMgr.reset();
    m_skill.init(m_player.getAircraftType());

    syncMap();
    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_SCORE);
    fireChange(PROP_ID_LIVES);
    fireChange(PROP_ID_GAME_STATE);
    fireChange(PROP_ID_SKILL_COOLDOWN);
    fireChange(PROP_ID_WEAPON_LEVEL);
    fireChange(PROP_ID_WAVE);
    fireChange(PROP_ID_BOSS_HEALTH);

    log("GameMapVM", "Level " + std::to_string(levelId) + " started");
}

void GameMapVM::selectLevelImpl(int levelId) {
    if (levelId < 1 || levelId > 7) return;
    m_currentLevel = levelId;
    m_scoreMgr.setActiveHighScoreSlot(
        (m_mode == GameMode::Endless) ? 7 : (levelId - 1));
    log("GameMapVM", "Level " + std::to_string(levelId) + " selected, waiting for aircraft");
}

void GameMapVM::quitLevelImpl() {
    if (m_mode == GameMode::Campaign) {
        m_state = GameState::LevelSelect;
    } else {
        m_state = GameState::ModeSelect;
    }
    m_enemies.clear();
    m_bullets.clear();
    m_player.reset();
    m_elapsed = 0.0f;
    fireChange(PROP_ID_GAME_STATE);
    fireChange(PROP_ID_MAP);
    log("GameMapVM", "Quit level - back to select");
}

void GameMapVM::navigateImpl(int state) {
    m_state = static_cast<GameState>(state);
    fireChange(PROP_ID_GAME_STATE);
}

void GameMapVM::upgradeStatImpl(int type) {
    auto ut = static_cast<UpgradeType>(type);
    if (m_upgradeMgr.upgrade(ut)) {
        m_player.setUpgradeBonuses(
            m_upgradeMgr.getFirePowerBonus(),
            m_upgradeMgr.getLivesBonus(),
            m_upgradeMgr.getSpeedBonus(),
            m_upgradeMgr.getCooldownBonus()
        );
        fireChange(PROP_ID_UPGRADE_LEVELS);
        fireChange(PROP_ID_STAR_CORES);
        fireChange(PROP_ID_LIVES);
        // 持久化升级数据
        int pp[5]; m_upgradeMgr.packAllLevels(pp);
        emit saveUpgradeRequested(m_upgradeMgr.getStarCores(),
                                  pp[0], pp[1], pp[2], pp[3], pp[4]);
        log("GameMapVM", "Upgrade stat " + std::to_string(type) + " to level " +
            std::to_string(m_upgradeMgr.getUpgradeLevel(ut)));
    }
}

void GameMapVM::setSlotHighScore(int slot, int hs) noexcept {
    if (slot >= 0 && slot < 8) {
        m_scoreMgr.setHighScore(slot, hs);
        if (slot == ((m_mode == GameMode::Endless) ? 7 : (m_currentLevel - 1)))
            m_scoreMgr.setActiveHighScoreSlot(slot);
    }
}

void GameMapVM::resetAllImpl() {
    for (int s = 0; s < 8; ++s) m_scoreMgr.setHighScore(s, 0);
    m_scoreMgr.setActiveHighScoreSlot(0);
    static const int zero[5] = {};
    m_upgradeMgr.setAllLevelsFromArray(zero);
    m_upgradeMgr.setCurrentAircraft(m_upgradeMgr.getCurrentAircraft());  // 刷新缓存
    m_upgradeMgr.addStarCores(-m_upgradeMgr.getStarCores());
    m_player.setUpgradeBonuses(0, 0, 0, 0);
    int packed[5]; m_upgradeMgr.packAllLevels(packed);
    emit saveUpgradeRequested(0, packed[0], packed[1], packed[2], packed[3], packed[4]);
    emit saveHighScoreRequested(0, 0);
    emit resetAllRequested();
    fireChange(PROP_ID_UPGRADE_LEVELS);
    fireChange(PROP_ID_STAR_CORES);
    fireChange(PROP_ID_MAX_UNLOCKED_LEVEL);
    log("GameMapVM", "All save data reset");
}

void GameMapVM::initUpgradeData(int starCores, const int packedLevels[5]) {
    m_upgradeMgr.addStarCores(starCores);
    m_upgradeMgr.setAllLevelsFromArray(packedLevels);
    m_upgradeMgr.setCurrentAircraft(m_upgradeMgr.getCurrentAircraft());  // 刷新缓存
    m_player.setUpgradeBonuses(
        m_upgradeMgr.getFirePowerBonus(),
        m_upgradeMgr.getLivesBonus(),
        m_upgradeMgr.getSpeedBonus(),
        m_upgradeMgr.getCooldownBonus()
    );
    log("GameMapVM", "Upgrade data loaded: cores=" + std::to_string(starCores)
        + " lv0=" + std::to_string(packedLevels[0]));
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
    m_aircraftNameBuf = AircraftStats::getTemplate(static_cast<AircraftType>(type)).name;
    m_upgradeMgr.setCurrentAircraft(type);
    m_player.setUpgradeBonuses(
        m_upgradeMgr.getFirePowerBonus(),
        m_upgradeMgr.getLivesBonus(),
        m_upgradeMgr.getSpeedBonus(),
        m_upgradeMgr.getCooldownBonus()
    );
    fireChange(PROP_ID_UPGRADE_LEVELS);
}

void GameMapVM::useSkillImpl() {
    if (m_state != GameState::Playing) return;
    if (!m_skill.activate()) return;

    const auto& tmpl = AircraftStats::getTemplate(m_player.getAircraftType());
    switch (tmpl.skill) {
    case SkillType::ThunderStrike:
        handleThunderStrike();
        m_thunderActive = true;
        break;
    case SkillType::FrostShield:   break;
    case SkillType::IronWall:      break;
    case SkillType::FlameStorm:    break;
    case SkillType::TimeDash:      break;
    }
    // 所有技能激活期间无敌
    m_player.setSkillInvincible(true);
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

void GameMapVM::applySkillEffects(float dt) {
    if (!m_skill.isActive()) {
        m_flameStormTimer = 0.0f;
        m_isDashing = false;
        m_player.setSkillInvincible(false);  // 技能结束，清除无敌
        return;
    }
    const auto& tmpl = AircraftStats::getTemplate(m_player.getAircraftType());
    switch (tmpl.skill) {
    case SkillType::FlameStorm: handleFlameStorm(dt); break;
    case SkillType::TimeDash:   handleTimeDash(dt);   break;
    case SkillType::IronWall:   handleIronWall(dt);   break;
    default: break;
    }
}

void GameMapVM::handleThunderStrike() {
    m_thunderActive = true;   // 迭代7：通知 View 绘制雷击特效
    m_thunderTimer = 0.3f;
    for (auto& e : m_enemies) {
        for (int i = 0; i < 5; ++i) {
            if (!e->isDead()) e->takeDamage();
        }
        if (e->isDead()) {
            m_explosionQueue.push_back(e->getPos());
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
        m_dashTimer = 0.5f;  // 延长冲刺时间，特效更明显
        int hitCount = 0;
        for (auto& e : m_enemies) {
            if (e->isDead()) continue;
            float ex = e->getPos().x, ey = e->getPos().y;
            // 前方扇形范围：上下 0.8、左右 0.25
            if (ey < m_player.getPos().y && ey > m_player.getPos().y - 0.8f &&
                std::abs(ex - m_player.getPos().x) < 0.25f) {
                e->takeDamage();
                if (e->isDead()) {
                    m_explosionQueue.push_back(e->getPos());
                    m_powerUpMgr.onEnemyDestroyed(e->getPos(), m_rng);
                } else {
                    // 没死的再补一刀
                    e->takeDamage();
                }
                hitCount++;
            }
        }
        log("GameMapVM", "TimeDash hit " + std::to_string(hitCount) + " enemies");
    }
    // 冲刺期间持续发射前方快速弹幕
    if (m_isDashing) {
        m_flameStormTimer += dt;
        if (m_flameStormTimer >= 0.08f) {
            m_flameStormTimer = 0.0f;
            Vec2 p = m_player.getPos();
            for (int i = -1; i <= 1; ++i) {
                m_bullets.emplace_back(
                    p.x + i * 0.025f, p.y - m_player.getSize(),
                    i * 0.2f, -BULLET_SPEED * 1.5f, Bullet::Player);
            }
        }
    }
    m_dashTimer -= dt;
    if (m_dashTimer <= 0.0f) m_isDashing = false;
}

// ── 铁壁阵：持续期间自动向四周发射反击弹幕 ─────────────────────
void GameMapVM::handleIronWall(float dt) {
    m_flameStormTimer += dt;
    if (m_flameStormTimer >= 0.15f) {
        m_flameStormTimer = 0.0f;
        Vec2 p = m_player.getPos();
        for (int i = 0; i < 8; ++i) {
            float angle = 3.14159f * 2.0f * i / 8.0f;
            m_bullets.emplace_back(
                p.x + std::cos(angle) * PLAYER_SIZE * 0.5f,
                p.y + std::sin(angle) * PLAYER_SIZE * 0.5f,
                std::cos(angle) * BULLET_SPEED * 0.6f,
                std::sin(angle) * BULLET_SPEED * 0.6f,
                Bullet::Player);
        }
    }
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
                    m_scoreMgr.onEnemyKilled();  // 迭代7：统计
                    m_scoreMgr.addScore(m_enemies[ei]->getScore());
                    // 星核掉落
                    m_upgradeMgr.addStarCores(STAR_CORE_PER_KILL);
                    fireChange(PROP_ID_STAR_CORES);
                    // 爆炸粒子位置
                    m_explosionQueue.push_back(m_enemies[ei]->getPos());
                    m_powerUpMgr.onEnemyDestroyed(m_enemies[ei]->getPos(), m_rng);
                }
                m_bullets[bi].markDead();
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
            b.markDead();
            if (m_player.isDead()) {
                m_state = GameState::GameOver;
                log("GameMapVM", "Player died - Game Over");
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
                log("GameMapVM", "Player died - Game Over");
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
        return b.isOffScreen() || b.isDead();
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
        if (b.isDead()) continue; // 已标记移除
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
        case PowerUpType::StarCore: a.type = ActorType::PowerUpStarCore; break;
        }
        a.x = pu.pos.x;
        a.y = pu.pos.y;
        a.hp = 1; a.maxHp = 1;
        m_map.append(a);
    }

    // 爆炸标记（View 读取后生成粒子特效，每帧自动清空）
    // 同时写入 float 数组供 View 直接读取（不受 AirMap 异步绘制 timing 影响）
    m_explosionCount = 0;
    for (const auto& ep : m_explosionQueue) {
        Actor a;
        a.type  = ActorType::Explosion;
        a.x     = ep.x;
        a.y     = ep.y;
        a.hp    = 1; a.maxHp = 1;
        m_map.append(a);
        // 同步写入 float 数组
        if (m_explosionCount < MAX_EXPLOSION_DATA) {
            m_explosionData[m_explosionCount * 2 + 0] = ep.x;
            m_explosionData[m_explosionCount * 2 + 1] = ep.y;
            m_explosionCount++;
        }
    }
}

void GameMapVM::fireChange(uint32_t id) {
    emit propertyChanged(id);
}
