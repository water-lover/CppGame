#include "app/AppAgent.hpp"
#include "view/GameView.hpp"
#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/SpiritVM.hpp"
#include "resource/AssetManager.hpp"
#include "resource/SaveManager.hpp"
#include "common/Logger.hpp"

#include <QApplication>

// ═══════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════

AppAgent::AppAgent() = default;

AppAgent::~AppAgent() = default;

// ═══════════════════════════════════════════════════════════════════
// 初始化 — 创建所有 Agent + 建立三绑定
// ═══════════════════════════════════════════════════════════════════

void AppAgent::init() {
    log("AppAgent", "=== Thunder Fighter 初始化 ===");

    // ── 1. 创建所有 Agent 实例 ──────────────────────────────────
    m_mapVM    = new GameMapVM();
    m_spriteVM = new SpiritVM();
    m_gameView = new GameView();          // GameView 内置 QTimer 驱动帧循环
    m_gameView->setWindowTitle(QStringLiteral("雷霆战机 — Thunder Fighter"));

    // ── 2. 通过 Resource Agent 加载所有精灵图片 ────────────────
    //    App 是唯一接触 Resource 的层，ViewModel 不直接调用 AssetManager
    AssetManager& assets = AssetManager::instance();
    const QPixmap* playerPixmap   = assets.getImage("playerShip");
    const QPixmap* enemyPixmap    = assets.getImage("enemySmall");
    const QPixmap* bulletPixmap   = assets.getImage("playerBullet");
    const QPixmap* enemyBulletPix = assets.getImage("enemyBullet");
    const QPixmap* bgPixmap       = assets.getImage("background");

    if (!playerPixmap)  log("AppAgent", "WARNING: playerShip image not found");
    if (!enemyPixmap)   log("AppAgent", "WARNING: enemySmall image not found");
    if (!bulletPixmap)  log("AppAgent", "WARNING: playerBullet image not found");
    if (!bgPixmap)      log("AppAgent", "WARNING: background image not found");

    // ── 3. 将图片注入 SpiritVM（ViewModel 层，作为图片中介） ──
    m_spriteVM->setPlayerPixmap(playerPixmap);
    m_spriteVM->setEnemySmallPixmap(enemyPixmap);
    m_spriteVM->setPlayerBulletPixmap(bulletPixmap);
    m_spriteVM->setEnemyBulletPixmap(enemyBulletPix);
    m_spriteVM->setBackgroundPixmap(bgPixmap);

    // ═══════════════════════════════════════════════════════════════
    // ① 属性绑定 — 将 ViewModel 的 const T* 指针注入 View
    //    View 只读访问，完全不认识 ViewModel
    // ═══════════════════════════════════════════════════════════════

    m_gameView->setMap(m_mapVM->getMap());
    m_gameView->setPlayerPixmap(playerPixmap);
    m_gameView->setEnemySmallPixmap(enemyPixmap);
    m_gameView->setBulletPixmap(bulletPixmap);
    m_gameView->setBackgroundPixmap(bgPixmap);

    // 简单值通过桥接变量暴露为 const T*（GameMapVM 返回 int 而非 const int*）
    m_bridgeScore     = m_mapVM->getScore();
    m_bridgeLives     = m_mapVM->getLives();
    m_bridgeHighScore = m_mapVM->getHighScore();
    m_bridgeState     = m_mapVM->getGameState();

    m_gameView->setScorePtr(&m_bridgeScore);
    m_gameView->setLivesPtr(&m_bridgeLives);
    m_gameView->setHighScorePtr(&m_bridgeHighScore);
    m_gameView->setGameStatePtr(&m_bridgeState);

    // ═══════════════════════════════════════════════════════════════
    // ② 命令绑定 — 将 ViewModel 的 std::function 命令注入 View
    //    View 调用命令时不知道具体实现者
    // ═══════════════════════════════════════════════════════════════

    m_gameView->setTickCommand(m_mapVM->getTickCommand());
    m_gameView->setMoveUpCommand(m_mapVM->getMoveUpCommand());
    m_gameView->setMoveDownCommand(m_mapVM->getMoveDownCommand());
    m_gameView->setMoveLeftCommand(m_mapVM->getMoveLeftCommand());
    m_gameView->setMoveRightCommand(m_mapVM->getMoveRightCommand());
    m_gameView->setStartGameCommand(m_mapVM->getStartGameCommand());

    // ═══════════════════════════════════════════════════════════════
    // ③ 事件绑定 — ViewModel 的 signal → View 的 slot
    //    App 中转连接，ViewModel 和 View 互不知道对方
    // ═══════════════════════════════════════════════════════════════

    // App 先接收 propertyChanged → 更新桥接变量（确保 View 读取时已是最新值）
    QObject::connect(m_mapVM, &GameMapVM::propertyChanged,
                     [this](uint32_t id) { onViewModelChanged(id); });

    // View 再接收 propertyChanged → 此时桥接变量已更新，读到正确值
    QObject::connect(m_mapVM, &GameMapVM::propertyChanged,
                     m_gameView, &GameView::onPropertyChanged);

    log("AppAgent", "三绑定建立完成");
    log("AppAgent", "=== 初始化完成 ===");
}

// ═══════════════════════════════════════════════════════════════════
// 事件通知中转 — 更新桥接变量 + 其他跨层操作
// ═══════════════════════════════════════════════════════════════════

void AppAgent::onViewModelChanged(uint32_t propertyId) {
    switch (propertyId) {
    case PROP_ID_SCORE:
        m_bridgeScore = m_mapVM->getScore();
        break;

    case PROP_ID_LIVES:
        m_bridgeLives = m_mapVM->getLives();
        break;

    case PROP_ID_GAME_STATE:
        m_bridgeState = m_mapVM->getGameState();
        if (m_bridgeState == GameState::GameOver) {
            // 游戏结束 → 持久化最高分
            int finalScore = m_mapVM->getScore();
            int highScore  = m_mapVM->getHighScore();
            m_bridgeHighScore = highScore;

            SaveManager saveMgr;
            saveMgr.saveHighScore(highScore);
            log("AppAgent", "Game Over — Score: " + std::to_string(finalScore)
                + " HighScore: " + std::to_string(highScore));
        }
        break;

    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 运行
// ═══════════════════════════════════════════════════════════════════

int AppAgent::run() {
    // 显示主窗口
    m_gameView->show();

    log("AppAgent", "进入事件循环");
    return qApp->exec();
}
