#ifndef GAMEMAPVM_HPP
#define GAMEMAPVM_HPP

#include <QObject>
#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "common/AirMap.hpp"
#include "common/Constants.hpp"
#include "common/PropertyIds.hpp"
#include "common/Types.hpp"
#include "viewmodel/AircraftStats.hpp"
#include "viewmodel/Bullet.hpp"
#include "viewmodel/Enemy.hpp"
#include "viewmodel/Player.hpp"
#include "viewmodel/ScoreManager.hpp"
#include "viewmodel/SkillSystem.hpp"
#include "viewmodel/WaveManager.hpp"
#include "viewmodel/PowerUpManager.hpp"

/// 游戏地图 ViewModel — 核心 Function Model
///
/// 三绑定：
///   ① 属性绑定：const AirMap* + 简单值 getter
///   ② 命令绑定：std::function getter，由 App 注入 View
///   ③ 事件绑定：emit propertyChanged(uint32_t)
///
/// 迭代 3 新增：
///   - 战机选择（5选1）
///   - 技能系统（Space 释放，冷却管理）
///   - 武器等级（1~5）
class GameMapVM : public QObject {
    Q_OBJECT

public:
    explicit GameMapVM(QObject* parent = nullptr);
    ~GameMapVM() override = default;

    // ════════════════════════════════════════════════════════════════
    // ① 属性绑定（ViewModel → View）
    // ════════════════════════════════════════════════════════════════

    const AirMap* getMap()        const noexcept { return &m_map; }
    int   getScore()              const noexcept { return m_scoreMgr.getScore(); }
    int   getLives()              const noexcept { return m_player.getLives(); }
    int   getHighScore()          const noexcept { return m_scoreMgr.getHighScore(); }
    GameState getGameState()      const noexcept { return m_state; }
    int   getScoreValue()         const noexcept { return m_scoreMgr.getScore(); }
    int   getLivesValue()         const noexcept { return m_player.getLives(); }
    int   getHighScoreValue()     const noexcept { return m_scoreMgr.getHighScore(); }

    // 迭代 3 新属性
    int   getAircraftType()       const noexcept { return static_cast<int>(m_player.getAircraftType()); }
    const char* getAircraftName() const noexcept {
        return AircraftStats::getTemplate(m_player.getAircraftType()).name;
    }
    int   getWeaponLevel()        const noexcept { return m_player.getWeaponLevel(); }
    float getSkillCooldownPercent() const noexcept { return m_skill.getCooldownPercent(); }
    bool  isSkillReady()          const noexcept { return !m_skill.isOnCooldown(); }
    bool  isSkillActive()         const noexcept { return m_skill.isActive(); }

    // 迭代 3：波次 / BOSS 属性（供 App 桥接至 View）
    GameMode getGameMode()        const noexcept { return m_mode; }
    int   getCurrentLevel()       const noexcept { return m_currentLevel; }
    int   getWave()               const noexcept { return m_wave; }
    int   getBossHp()             const noexcept { return m_bossHp; }
    int   getBossMaxHp()          const noexcept { return m_bossMaxHp; }

    // ════════════════════════════════════════════════════════════════
    // ② 命令绑定（View → ViewModel）
    // ════════════════════════════════════════════════════════════════

    std::function<void()>         getStartGameCommand();
    std::function<void(int)>      getMoveUpCommand();
    std::function<void(int)>      getMoveDownCommand();
    std::function<void(int)>      getMoveLeftCommand();
    std::function<void(int)>      getMoveRightCommand();
    std::function<void(float)>    getTickCommand();
    std::function<void(int)>      getSelectModeCommand();
    std::function<void()>         getPauseCommand();

    // 迭代 3 新命令
    std::function<void(int)>      getSelectAircraftCommand();   ///< 选择战机 (int AircraftType)
    std::function<void()>         getUseSkillCommand();         ///< 释放技能 (Space)

signals:
    // ════════════════════════════════════════════════════════════════
    // ③ 事件绑定（ViewModel → View）
    // ════════════════════════════════════════════════════════════════
    void propertyChanged(uint32_t propertyId);

private:
    // ── 命令实现 ──────────────────────────────────────────────────
    void startGameImpl();
    void tickImpl(float dt);
    void moveUpImpl(int active);
    void moveDownImpl(int active);
    void moveLeftImpl(int active);
    void moveRightImpl(int active);
    void selectModeImpl(int mode);
    void pauseImpl();
    void selectAircraftImpl(int type);
    void useSkillImpl();

    // ── 内部工具 ──────────────────────────────────────────────────
    void spawnEnemy();
    void checkCollisions();
    void handleEnemyAttacks(float dt);  // 迭代3：敌机攻击
    void applySkillEffects();       // 迭代3：处理技能效果（持续型）
    void handleThunderStrike();     // 全屏雷击
    void handleFlameStorm(float dt);// 扇形火焰
    void handleTimeDash(float dt);  // 冲刺攻击
    void cleanupEntities();
    void syncMap();
    void fireChange(uint32_t id);

    // ── 数据成员 ──────────────────────────────────────────────────
    GameState m_state = GameState::Menu;
    GameMode  m_mode  = GameMode::Endless;

    Player                      m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<Bullet>                 m_bullets;
    ScoreManager                m_scoreMgr;
    SkillSystem                 m_skill;        // 迭代 3
    WaveManager                 m_waveMgr;      // 迭代 3
    PowerUpManager              m_powerUpMgr;   // 迭代 3
    AirMap                      m_map;

    float m_spawnTimer  = 0.0f;
    float m_elapsed     = 0.0f;

    int   m_lastScore   = 0;
    int   m_lastLives   = 0;
    bool  m_lastGameOver = false;
    float m_lastSkillCD  = 0.0f;    // 迭代 3
    int   m_lastWeaponLv = 1;       // 迭代 3

    // 迭代 3：波次 / BOSS 数据
    int   m_currentLevel = 1;
    int   m_wave         = 0;
    int   m_bossHp       = 0;
    int   m_bossMaxHp    = 0;

    // 烈焰号火焰风暴：额外子弹发射计时
    float m_flameStormTimer = 0.0f;

    // 幻影号时空闪避：冲刺状态
    bool  m_isDashing    = false;
    float m_dashTimer    = 0.0f;

    std::mt19937 m_rng;
};

#endif // GAMEMAPVM_HPP
