#ifndef GAMEVIEW_HPP
#define GAMEVIEW_HPP

#include <QWidget>
#include <QTimer>
#include <QStackedWidget>
#include <QKeyEvent>
#include <functional>
#include <cstdint>

class QGraphicsView;
class QGraphicsScene;
class QPixmap;

#include "common/AirMap.hpp"
#include "common/PropertyIds.hpp"
#include "common/Types.hpp"
#include "view/GameScene.hpp"

class GameScene;
class StartScreen;
class GameOverScreen;
class ModeSelectScreen;
class PauseOverlay;
class BossHealthBar;
class LevelSelectScreen;
class AircraftSelectScreen;

/// 游戏主视图 — 纯 C++ QWidget
///
/// 职责：
///   - 顶层容器，内含 QStackedWidget 实现界面切换
///   - 拥有 QTimer 驱动帧循环（～60 FPS）
///   - 捕获键盘输入，通过 std::function 命令转发给 ViewModel
///   - 接收 ViewModel 的事件通知（propertyChanged）
///
/// 严格 MVVM 三绑定：
///   ① 属性绑定 — 通过 setter 接收 const T* 指针（只读）
///   ② 命令绑定 — 通过 setter 接收 std::function 命令
///   ③ 事件绑定 — 通过 onPropertyChanged(uint32_t) 槽函数
class GameView : public QWidget {
    Q_OBJECT

public:
    explicit GameView(QWidget* parent = nullptr);
    ~GameView() override;

    // ════════════════════════════════════════════════════════════════
    // ① 属性绑定 — 接收数据指针（由 App 注入）
    // ════════════════════════════════════════════════════════════════

    /// 设置精灵集合指针（只读，同时转发给 GameScene）
    void setMap(const AirMap* map) noexcept;

    /// 设置玩家飞机图片
    void setPlayerPixmap(const QPixmap* p) noexcept;

    /// 设置小型敌机图片
    void setEnemySmallPixmap(const QPixmap* p) noexcept;

    /// 设置玩家子弹图片
    void setBulletPixmap(const QPixmap* p) noexcept;

    /// 设置背景图片
    void setBackgroundPixmap(const QPixmap* p) noexcept;

    /// 迭代3 新图片
    void setEnemyMediumPixmap(const QPixmap* p) noexcept;
    void setEnemyLargePixmap(const QPixmap* p) noexcept;
    void setBossPixmap(const QPixmap* p) noexcept;
    void setBossPixmap2(const QPixmap* p) noexcept;
    void setBossPixmap3(const QPixmap* p) noexcept;
    void setBossPixmap4(const QPixmap* p) noexcept;
    void setEnemyBulletPixmap(const QPixmap* p) noexcept;
    void setPowerUpHpPixmap(const QPixmap* p) noexcept;
    void setPowerUpFirePixmap(const QPixmap* p) noexcept;
    void setPowerUpShieldPixmap(const QPixmap* p) noexcept;

    // 分数/生命指针（转发给 GameScene 直接在场景中绘制 HUD）
    void setScorePtr(const int* p)   noexcept { m_pScore = p; if (m_scene) m_scene->setHudScore(p); }
    void setLivesPtr(const int* p)   noexcept { m_pLives = p; if (m_scene) m_scene->setHudLives(p); }
    void setHighScorePtr(const int* p) noexcept { m_pHighScore = p; if (m_scene) m_scene->setHudHighScore(p); }
    void setGameStatePtr(const GameState* p) noexcept { m_pGameState = p; }
    void setWavePtr(const int* p)    noexcept { m_pWave = p; if (m_scene) m_scene->setHudWave(p); }
    void setBossHpPtr(const int* p)    noexcept { m_pBossHp = p; }
    void setBossMaxHpPtr(const int* p) noexcept { m_pBossMaxHp = p; }
    void setLevelClearedPtr(const bool* p) noexcept { m_pLevelCleared = p; }
    void setCurrentLevelPtr(const int* p) noexcept { m_pCurrentLevel = p; }
    void setSkillCooldownPtr(const float* p) noexcept { m_pSkillCD = p; if (m_scene) m_scene->setHudSkillCooldown(p); }
    void setSkillReadyPtr(const bool* p) noexcept { m_pSkillReady = p; if (m_scene) m_scene->setHudSkillReady(p); }
    void setSkillActivePtr(const bool* p) noexcept { m_pSkillActive = p; if (m_scene) m_scene->setHudSkillActive(p); }
    void setSkillTypePtr(const int* p) noexcept { if (m_scene) m_scene->setHudSkillType(p); }
    void setHasShieldPtr(const bool* p) noexcept { if (m_scene) m_scene->setHudHasShield(p); }
    void setAircraftNamePtr(const char* p) noexcept { if (m_scene) m_scene->setHudAircraftName(p); }

    // ════════════════════════════════════════════════════════════════
    // ② 命令绑定 — 接收 std::function 命令（由 App 注入）
    // ════════════════════════════════════════════════════════════════

    void setTickCommand(std::function<void(float)>&& cmd);
    void setMoveUpCommand(std::function<void(int)>&& cmd);
    void setMoveDownCommand(std::function<void(int)>&& cmd);
    void setMoveLeftCommand(std::function<void(int)>&& cmd);
    void setMoveRightCommand(std::function<void(int)>&& cmd);
    void setStartGameCommand(std::function<void()>&& cmd);
    void setSelectModeCommand(std::function<void(int)>&& cmd);
    void setPauseCommand(std::function<void()>&& cmd);
    void setStartLevelCommand(std::function<void(int)>&& cmd);
    void setSelectLevelCommand(std::function<void(int)>&& cmd);
    void setQuitLevelCommand(std::function<void()>&& cmd);
    void setSelectAircraftCommand(std::function<void(int)>&& cmd);
    void setUseSkillCommand(std::function<void()>&& cmd);
    void setNavigateCommand(std::function<void(int)>&& cmd);
    void setLevelSelectMaxUnlocked(int level) noexcept;

    // ════════════════════════════════════════════════════════════════
    // ③ 事件绑定 — 接收 ViewModel 通知
    // ════════════════════════════════════════════════════════════════

public slots:
    /// ViewModel 数据变化时调用（由 App 的 connect 中转）
    void onPropertyChanged(uint32_t propertyId);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    /// 帧循环回调
    void tick();

    /// 根据游戏状态切换界面
    void updatePage();

    // ── 帧循环 ────────────────────────────────────────────────────
    QTimer* m_timer = nullptr;

    // ── 界面栈 ────────────────────────────────────────────────────
    QStackedWidget* m_pageStack = nullptr;

    // 页面 0: 开始界面
    StartScreen*       m_startScreen = nullptr;
    // 页面 1: 战机选择界面
    AircraftSelectScreen* m_aircraftSelectScreen = nullptr;
    // 页面 2: 模式选择界面
    ModeSelectScreen*  m_modeSelectScreen = nullptr;
    // 页面 3: 关卡选择界面
    LevelSelectScreen* m_levelSelectScreen = nullptr;
    // 页面 4: 游戏页面
    QWidget*        m_gamePage = nullptr;
    QGraphicsView*  m_graphicsView = nullptr;
    GameScene*      m_scene = nullptr;
    // 页面 5: 游戏结束界面
    GameOverScreen* m_gameOverScreen = nullptr;
    // 页面 6: 暂停覆盖层
    PauseOverlay*   m_pauseOverlay = nullptr;

    // ── BOSS 血条（叠加在游戏页面上方） ───────────────────────
    BossHealthBar*  m_bossHealthBar = nullptr;

    // ── 属性指针（const T* 只读） ────────────────────────────────
    const AirMap*   m_pMap        = nullptr;
    const QPixmap*  m_pPlayerImg  = nullptr;
    const QPixmap*  m_pEnemyImg   = nullptr;
    const QPixmap*  m_pBulletImg  = nullptr;
    const QPixmap*  m_pBgImg      = nullptr;
    const int*      m_pScore      = nullptr;
    const int*      m_pLives      = nullptr;
    const int*      m_pHighScore  = nullptr;
    const int*      m_pWave       = nullptr;
    const int*      m_pBossHp     = nullptr;
    const int*      m_pBossMaxHp  = nullptr;
    const GameState* m_pGameState = nullptr;
    const bool*     m_pLevelCleared = nullptr;
    const int*      m_pCurrentLevel = nullptr;

    // ── 命令（std::function，不知道实现者） ──────────────────────
    std::function<void(float)> m_tickCommand;
    std::function<void(int)>   m_moveUpCommand;
    std::function<void(int)>   m_moveDownCommand;
    std::function<void(int)>   m_moveLeftCommand;
    std::function<void(int)>   m_moveRightCommand;
    std::function<void()>      m_startGameCommand;
    std::function<void(int)>   m_selectModeCommand;
    std::function<void()>      m_pauseCommand;
    std::function<void(int)>   m_startLevelCommand;
    std::function<void(int)>   m_selectLevelCommand;
    std::function<void()>      m_quitLevelCommand;
    std::function<void(int)>   m_selectAircraftCommand;
    std::function<void()>      m_useSkillCommand;
    std::function<void(int)>   m_navigateCommand;
    // ── 技能 HUD 指针 ──────────────────────────────────────
    const float* m_pSkillCD     = nullptr;
    const bool*  m_pSkillReady  = nullptr;
    const bool*  m_pSkillActive = nullptr;
};

#endif // GAMEVIEW_HPP
