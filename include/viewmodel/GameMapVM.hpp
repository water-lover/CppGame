#ifndef GAMEMAPVM_HPP
#define GAMEMAPVM_HPP

#include <QObject>
#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "common/AirMap.hpp"
#include "viewmodel/GameConstants.hpp"
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
#include "viewmodel/UpgradeManager.hpp"

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

    // ── const T* 属性绑定指针（View 只读持有，无需 App 桥接） ──
    const int*      getScorePtr()        const noexcept { return &m_lastScore; }
    const int*      getLivesPtr()        const noexcept { return &m_lastLives; }
    const int*      getHighScorePtr()    const noexcept { return &m_cachedHighScore; }
    const GameState* getGameStatePtr()   const noexcept { return &m_state; }
    const int*      getWavePtr()         const noexcept { return &m_wave; }
    const char*     getWaveDisplayPtr()  const noexcept { return m_waveDisplayBuf.c_str(); }
    const int*      getBossHpPtr()       const noexcept { return &m_bossHp; }
    const int*      getBossMaxHpPtr()    const noexcept { return &m_bossMaxHp; }
    const int*      getCurrentLevelPtr() const noexcept { return &m_currentLevel; }
    const float*    getSkillCooldownPtr() const noexcept { return &m_lastSkillCD; }
    const bool*     isSkillReadyPtr()    const noexcept { return &m_lastSkillReady; }
    const bool*     isSkillActivePtr()   const noexcept { return &m_lastSkillActive; }
    const bool*     getHasShieldPtr()    const noexcept { return m_player.getHasShieldPtr(); }
    const int*      getSkillTypePtr()    const noexcept { return &m_lastSkillType; }
    const int*      getWeaponLevelPtr()  const noexcept { return &m_lastWeaponLv; }
    const bool*     isLevelClearedPtr()  const noexcept { return &m_levelCleared; }

    // 迭代 6：升级系统
    const int* getUpgradeStarCoresPtr()  const noexcept { return m_upgradeMgr.getStarCoresPtr(); }
    const int* getUpgradeFireLevelPtr()    const noexcept { return m_upgradeMgr.getFireLevelPtr(); }
    const int* getUpgradeLivesLevelPtr()   const noexcept { return m_upgradeMgr.getLivesLevelPtr(); }
    const int* getUpgradeSpeedLevelPtr()   const noexcept { return m_upgradeMgr.getSpeedLevelPtr(); }
    const int* getUpgradeCooldownLevelPtr() const noexcept { return m_upgradeMgr.getCooldownLevelPtr(); }
    void initUpgradeData(int starCores, int packedLevels);

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
    int   getMaxUnlockedLevel()   const noexcept { return m_maxUnlockedLevel; }
    void  setMaxUnlockedLevel(int lvl) noexcept { m_maxUnlockedLevel = lvl; }
    const int* getMaxUnlockedLevelPtr() const noexcept { return &m_maxUnlockedLevel; }
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

    // 迭代 4 新命令
    std::function<void(int)>      getStartLevelCommand();       ///< 开始指定关卡游戏
    std::function<void(int)>      getSelectLevelCommand();      ///< 选择关卡（不开始，等待选战机）
    std::function<void()>         getQuitLevelCommand();        ///< 退出关卡

    // UI 导航命令（让 ViewModel 控制页面切换，而非 View 直接操作页面栈）
    std::function<void(int)>      getNavigateCommand();

    // 迭代 6：升级命令
    std::function<void(int)>      getUpgradeStatCommand();

    // 迭代 4：通关胜利标记
    bool  isLevelCleared()  const noexcept { return m_levelCleared; }

signals:
    // ════════════════════════════════════════════════════════════════
    // ③ 事件绑定（ViewModel → View）
    // ════════════════════════════════════════════════════════════════
    void propertyChanged(uint32_t propertyId);

    // ── 持久化请求（ViewModel → App，App 再调 SaveManager）──
    void saveHighScoreRequested(int score);
    void saveCampaignRequested(int level);
    void saveUpgradeRequested(int starCores, int packedLevels);

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
    void startLevelImpl(int levelId);  // 迭代4：从选关界面开始指定关卡
    void selectLevelImpl(int levelId);  // 迭代5：仅存储关卡，等待选战机
    void quitLevelImpl();               // 迭代4：退出关卡
    void navigateImpl(int state);        // UI 导航

    // 迭代 6
    void upgradeStatImpl(int type);

    // ── 内部工具 ──────────────────────────────────────────────────
    void spawnEnemy();
    void checkCollisions();
    void handleEnemyAttacks(float dt);  // 迭代3：敌机攻击
    void applySkillEffects(float dt);       // 迭代3：处理技能效果（持续型）
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

    // 迭代 6：升级系统
    UpgradeManager              m_upgradeMgr;

    float m_spawnTimer  = 0.0f;
    float m_elapsed     = 0.0f;

    int   m_lastScore   = 0;
    int   m_lastLives   = 0;
    int   m_cachedHighScore = 0;
    bool  m_lastGameOver = false;
    float m_lastSkillCD  = 0.0f;    // 迭代 3
    bool  m_lastSkillReady  = true;
    bool  m_lastSkillActive = false;
    int   m_lastSkillType   = 0;
    int   m_lastWeaponLv = 1;       // 迭代 3

    // 迭代 3：波次 / BOSS 数据
    int   m_currentLevel = 1;
    int   m_wave         = 0;
    std::string m_waveDisplayBuf;

    // 迭代 4：关卡解锁进度
    int   m_maxUnlockedLevel = 1;
    bool  m_levelCleared     = false;   // 迭代4：通关胜利标记
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
