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

class GameScene;
class HudOverlay;
class StartScreen;
class GameOverScreen;

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

    // 分数/生命指针（HUD 直接读取）
    void setScorePtr(const int* p)   noexcept { m_pScore = p; }
    void setLivesPtr(const int* p)   noexcept { m_pLives = p; }
    void setHighScorePtr(const int* p) noexcept { m_pHighScore = p; }
    void setGameStatePtr(const GameState* p) noexcept { m_pGameState = p; }

    // ════════════════════════════════════════════════════════════════
    // ② 命令绑定 — 接收 std::function 命令（由 App 注入）
    // ════════════════════════════════════════════════════════════════

    void setTickCommand(std::function<void(float)>&& cmd);
    void setMoveUpCommand(std::function<void(int)>&& cmd);
    void setMoveDownCommand(std::function<void(int)>&& cmd);
    void setMoveLeftCommand(std::function<void(int)>&& cmd);
    void setMoveRightCommand(std::function<void(int)>&& cmd);
    void setStartGameCommand(std::function<void()>&& cmd);

    // ════════════════════════════════════════════════════════════════
    // ③ 事件绑定 — 接收 ViewModel 通知
    // ════════════════════════════════════════════════════════════════

public slots:
    /// ViewModel 数据变化时调用（由 App 的 connect 中转）
    void onPropertyChanged(uint32_t propertyId);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

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
    StartScreen*    m_startScreen = nullptr;
    // 页面 1: 游戏页面（含 QGraphicsView + HUD）
    QWidget*        m_gamePage = nullptr;
    QGraphicsView*  m_graphicsView = nullptr;
    GameScene*      m_scene = nullptr;
    HudOverlay*     m_hud = nullptr;
    // 页面 2: 游戏结束界面
    GameOverScreen* m_gameOverScreen = nullptr;

    // ── 属性指针（const T* 只读） ────────────────────────────────
    const AirMap*   m_pMap        = nullptr;
    const QPixmap*  m_pPlayerImg  = nullptr;
    const QPixmap*  m_pEnemyImg   = nullptr;
    const QPixmap*  m_pBulletImg  = nullptr;
    const QPixmap*  m_pBgImg      = nullptr;
    const int*      m_pScore      = nullptr;
    const int*      m_pLives      = nullptr;
    const int*      m_pHighScore  = nullptr;
    const GameState* m_pGameState = nullptr;

    // ── 命令（std::function，不知道实现者） ──────────────────────
    std::function<void(float)> m_tickCommand;
    std::function<void(int)>   m_moveUpCommand;
    std::function<void(int)>   m_moveDownCommand;
    std::function<void(int)>   m_moveLeftCommand;
    std::function<void(int)>   m_moveRightCommand;
    std::function<void()>      m_startGameCommand;
};

#endif // GAMEVIEW_HPP
