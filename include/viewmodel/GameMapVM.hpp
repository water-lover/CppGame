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
#include "viewmodel/Bullet.hpp"
#include "viewmodel/Enemy.hpp"
#include "viewmodel/Player.hpp"
#include "viewmodel/ScoreManager.hpp"

/// 游戏地图 ViewModel — 核心 Function Model
///
/// 遵循课件 MVFM 模式：
///   - "游戏类的数据即为可绘制对象，不需要转换，所以取消 Model 层，仅使用 ViewModel 层。"
///   - "使用 std::function 类实现命令模式。"
///
/// 三绑定：
///   ① 属性绑定：const AirMap* getMap() — View 通过 const 指针只读遍历精灵
///   ② 命令绑定：getMoveUpCommand() 等返回 std::function — 由 App 注入 View
///   ③ 事件绑定：emit propertyChanged(uint32_t) — App 中转给 View 的槽函数
class GameMapVM : public QObject {
    Q_OBJECT

public:
    explicit GameMapVM(QObject* parent = nullptr);
    ~GameMapVM() override = default;

    // ════════════════════════════════════════════════════════════════
    // ① 属性绑定（ViewModel → View）
    //    View 通过 const T* 指针只读读取数据
    // ════════════════════════════════════════════════════════════════

    /// 精灵集合 — View 遍历此集合绘制所有对象
    const AirMap* getMap() const noexcept { return &m_map; }

    /// 当前分数
    int getScore() const noexcept { return m_scoreMgr.getScore(); }

    /// 当前生命值
    int getLives() const noexcept { return m_player.getLives(); }

    /// 最高分
    int getHighScore() const noexcept { return m_scoreMgr.getHighScore(); }

    /// 当前游戏状态
    GameState getGameState() const noexcept { return m_state; }

    // ════════════════════════════════════════════════════════════════
    // ② 命令绑定（View → ViewModel）
    //    通过 std::function getter 暴露命令，由 App 注入给 View
    // ════════════════════════════════════════════════════════════════

    /// 开始新游戏
    std::function<void()> getStartGameCommand();

    /// 向上移动（int active: 1=按下, 0=松开）
    std::function<void(int)> getMoveUpCommand();

    /// 向下移动
    std::function<void(int)> getMoveDownCommand();

    /// 向左移动
    std::function<void(int)> getMoveLeftCommand();

    /// 向右移动
    std::function<void(int)> getMoveRightCommand();

    /// 帧更新（float dt: 增量时间）
    std::function<void(float)> getTickCommand();

    /// 选择游戏模式（int mode: 0=闯关, 1=无尽）
    std::function<void(int)> getSelectModeCommand();

    /// 暂停/继续
    std::function<void()> getPauseCommand();

    // 属性绑定辅助 — 供 View 直接读取的简单值（非指针形式）
    int   getScoreValue()   const noexcept { return m_scoreMgr.getScore(); }
    int   getLivesValue()   const noexcept { return m_player.getLives(); }
    int   getHighScoreValue() const noexcept { return m_scoreMgr.getHighScore(); }

signals:
    // ════════════════════════════════════════════════════════════════
    // ③ 事件绑定（ViewModel → View，通过 Qt signal）
    //    对齐老师 ex5 的 PropertyTrigger::fire(PROP_ID_XXX)
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

    // ── 内部工具 ──────────────────────────────────────────────────
    void spawnEnemy();
    void checkCollisions();
    void cleanupEntities();
    void syncMap();         // 将内部数据同步到 AirMap
    void fireChange(uint32_t id);  // 发射 propertyChanged + 日志

    // ── 数据成员 ──────────────────────────────────────────────────
    GameState m_state = GameState::Menu;

    Player                      m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<Bullet>                 m_bullets;
    ScoreManager                m_scoreMgr;
    AirMap                      m_map;

    float m_spawnTimer  = 0.0f;
    float m_elapsed     = 0.0f;

    int   m_lastScore   = 0;
    int   m_lastLives   = 0;
    bool  m_lastGameOver = false;

    std::mt19937 m_rng;
};

#endif // GAMEMAPVM_HPP
