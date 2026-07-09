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

    // ── 2. 从 Resource Agent 加载图片，注入 SpiritVM（图片中介层） ──
    AssetManager& assets = AssetManager::instance();

    // 5 架战机图片（通过 AircraftType 索引）
    m_spriteVM->setAircraftPixmap(AircraftType::Thunder,  assets.getImage("playerShip"));
    // 剩余战机图片待 QRC 扩展后注入
    // m_spriteVM->setAircraftPixmap(AircraftType::Flame,    assets.getImage("aircraftFlame"));
    // m_spriteVM->setAircraftPixmap(AircraftType::Frost,    assets.getImage("aircraftFrost"));
    // m_spriteVM->setAircraftPixmap(AircraftType::Phantom,  assets.getImage("aircraftSpeed"));
    // m_spriteVM->setAircraftPixmap(AircraftType::Fortress, assets.getImage("aircraftDefense"));

    m_spriteVM->setEnemySmallPixmap(assets.getImage("enemySmall"));
    m_spriteVM->setPlayerBulletPixmap(assets.getImage("playerBullet"));
    m_spriteVM->setEnemyBulletPixmap(assets.getImage("enemyBullet"));
    m_spriteVM->setBackgroundPixmap(assets.getImage("background"));

    // ── 迭代 3 新图片：先注入 SpiritVM（图片中介层） ─────
    m_spriteVM->setEnemyMediumPixmap(assets.getImage("enemyMedium"));
    m_spriteVM->setEnemyLargePixmap(assets.getImage("enemyLarge"));
    m_spriteVM->setBossPixmap(assets.getImage("bossShip"));
    m_spriteVM->setEnemyBulletPixmap(assets.getImage("enemyBullet"));
    m_spriteVM->setPowerUpHpPixmap(assets.getImage("powerUpHp"));
    m_spriteVM->setPowerUpFirePixmap(assets.getImage("powerUpFire"));
    m_spriteVM->setPowerUpShieldPixmap(assets.getImage("powerUpShield"));

    // ═══════════════════════════════════════════════════════════════
    // ① 属性绑定 — 从 SpiritVM 获取 const QPixmap* 注入 View
    //    View 只读访问，完全不认识 SpiritVM
    // ═══════════════════════════════════════════════════════════════

    m_gameView->setMap(m_mapVM->getMap());
    m_gameView->setPlayerPixmap(m_spriteVM->getAircraftPixmap(AircraftType::Thunder));
    m_gameView->setEnemySmallPixmap(m_spriteVM->getEnemySmallPixmap());
    m_gameView->setBulletPixmap(m_spriteVM->getPlayerBulletPixmap());
    m_gameView->setBackgroundPixmap(m_spriteVM->getBackgroundPixmap());

    // 迭代 3 新图片：从 SpiritVM getter 读取后注入 GameView
    m_gameView->setEnemyMediumPixmap(m_spriteVM->getEnemyMediumPixmap());
    m_gameView->setEnemyLargePixmap(m_spriteVM->getEnemyLargePixmap());
    m_gameView->setBossPixmap(m_spriteVM->getBossPixmap());
    m_gameView->setEnemyBulletPixmap(m_spriteVM->getEnemyBulletPixmap());
    m_gameView->setPowerUpHpPixmap(m_spriteVM->getPowerUpHpPixmap());
    m_gameView->setPowerUpFirePixmap(m_spriteVM->getPowerUpFirePixmap());
    m_gameView->setPowerUpShieldPixmap(m_spriteVM->getPowerUpShieldPixmap());

    // 简单值通过桥接变量暴露为 const T*（GameMapVM 返回 int 而非 const int*）
    m_bridgeScore     = m_mapVM->getScore();
    m_bridgeLives     = m_mapVM->getLives();
    m_bridgeHighScore = m_mapVM->getHighScore();
    m_bridgeWave      = 0;
    m_bridgeBossHp    = 0;
    m_bridgeBossMaxHp = 0;
    m_bridgeState     = m_mapVM->getGameState();

    m_gameView->setScorePtr(&m_bridgeScore);
    m_gameView->setLivesPtr(&m_bridgeLives);
    m_gameView->setHighScorePtr(&m_bridgeHighScore);
    m_gameView->setWavePtr(&m_bridgeWave);
    m_gameView->setBossHpPtr(&m_bridgeBossHp);
    m_gameView->setBossMaxHpPtr(&m_bridgeBossMaxHp);
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
    m_gameView->setSelectModeCommand(m_mapVM->getSelectModeCommand());
    m_gameView->setPauseCommand(m_mapVM->getPauseCommand());

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

            // 闯关模式：保存关卡进度
            if (m_mapVM->getGameMode() == GameMode::Campaign) {
                int level = m_mapVM->getCurrentLevel();
                saveMgr.saveCampaignProgress(level);
                log("AppAgent", "Campaign progress saved: level " + std::to_string(level));
            }

            log("AppAgent", "Game Over — Score: " + std::to_string(finalScore)
                + " HighScore: " + std::to_string(highScore));
        }
        break;

    case PROP_ID_BOSS_HEALTH:
        m_bridgeBossHp = m_mapVM->getBossHp();
        m_bridgeBossMaxHp = m_mapVM->getBossMaxHp();
        break;

    case PROP_ID_WAVE:
        m_bridgeWave = m_mapVM->getWave();
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
