#include "app/AppAgent.hpp"
#include "view/GameView.hpp"
#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/SpiritVM.hpp"
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
    m_gameView = new GameView();
    m_gameView->setWindowTitle(QStringLiteral("雷霆战机 — Thunder Fighter"));

    // ── 2. 初始化 SpiritVM（内部调用 AssetManager 加载图片） ───
    m_spriteVM->initialize();

    // ═══════════════════════════════════════════════════════════════
    // ① 属性绑定 — 从 SpiritVM/GameMapVM 获取 const T* 注入 View
    //    View 只读访问，完全不认识 SpiritVM/GameMapVM
    // ═══════════════════════════════════════════════════════════════

    // 精灵图片
    m_gameView->setMap(m_mapVM->getMap());
    m_gameView->setPlayerPixmap(m_spriteVM->getAircraftPixmap(AircraftType::Thunder));
    m_gameView->setEnemySmallPixmap(m_spriteVM->getEnemySmallPixmap());
    m_gameView->setBulletPixmap(m_spriteVM->getPlayerBulletPixmap());
    m_gameView->setBackgroundPixmap(m_spriteVM->getBackgroundPixmap());

    m_gameView->setEnemyMediumPixmap(m_spriteVM->getEnemyMediumPixmap());
    m_gameView->setEnemyLargePixmap(m_spriteVM->getEnemyLargePixmap());
    m_gameView->setBossPixmap(m_spriteVM->getBossPixmap());
    m_gameView->setBossPixmap2(m_spriteVM->getBossPixmapForHp(200));
    m_gameView->setBossPixmap3(m_spriteVM->getBossPixmapForHp(350));
    m_gameView->setBossPixmap4(m_spriteVM->getBossPixmapForHp(500));
    m_gameView->setEnemyBulletPixmap(m_spriteVM->getEnemyBulletPixmap());
    m_gameView->setPowerUpHpPixmap(m_spriteVM->getPowerUpHpPixmap());
    m_gameView->setPowerUpFirePixmap(m_spriteVM->getPowerUpFirePixmap());
    m_gameView->setPowerUpShieldPixmap(m_spriteVM->getPowerUpShieldPixmap());
    m_gameView->setPowerUpStarCorePixmap(m_spriteVM->getPowerUpStarCorePixmap());
    // GameMapVM 内部维护缓存，返回指向成员变量的稳定地址
    m_gameView->setScorePtr(m_mapVM->getScorePtr());
    m_gameView->setLivesPtr(m_mapVM->getLivesPtr());
    m_gameView->setHighScorePtr(m_mapVM->getHighScorePtr());
    m_gameView->setWavePtr(m_mapVM->getWavePtr());
    m_gameView->setBossHpPtr(m_mapVM->getBossHpPtr());
    m_gameView->setBossMaxHpPtr(m_mapVM->getBossMaxHpPtr());
    m_gameView->setGameStatePtr(m_mapVM->getGameStatePtr());
    m_gameView->setLevelClearedPtr(m_mapVM->isLevelClearedPtr());
    m_gameView->setCurrentLevelPtr(m_mapVM->getCurrentLevelPtr());
    m_gameView->setSkillCooldownPtr(m_mapVM->getSkillCooldownPtr());
    m_gameView->setSkillReadyPtr(m_mapVM->isSkillReadyPtr());
    m_gameView->setSkillActivePtr(m_mapVM->isSkillActivePtr());
    m_gameView->setSkillTypePtr(m_mapVM->getSkillTypePtr());
    m_gameView->setHasShieldPtr(m_mapVM->getHasShieldPtr());
    m_gameView->setAircraftNamePtr(m_mapVM->getAircraftName());

    // 迭代 6：星核指针注入（HUD 显示）
    m_gameView->setStarCoresPtr(m_mapVM->getUpgradeStarCoresPtr());
    // 升级等级指针注入（升级界面显示）
    m_gameView->setUpgradeFireLevelPtr(m_mapVM->getUpgradeFireLevelPtr());
    m_gameView->setUpgradeLivesLevelPtr(m_mapVM->getUpgradeLivesLevelPtr());
    m_gameView->setUpgradeSpeedLevelPtr(m_mapVM->getUpgradeSpeedLevelPtr());
    m_gameView->setUpgradeCooldownLevelPtr(m_mapVM->getUpgradeCooldownLevelPtr());

    // 初始关卡解锁（测试阶段全解锁）
    m_mapVM->setMaxUnlockedLevel(7);
    m_gameView->setLevelSelectMaxUnlocked(7);
    log("AppAgent", "All levels unlocked for testing (7/7)");

    // 迭代 6：从存档读取升级数据
    {
        auto upgradeData = SaveManager().loadUpgradeData();
        m_mapVM->initUpgradeData(upgradeData.starCores, upgradeData.levelsPacked);
        log("AppAgent", "Upgrade data loaded: cores=" + std::to_string(upgradeData.starCores));
    }

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
    m_gameView->setStartLevelCommand(m_mapVM->getStartLevelCommand());
    m_gameView->setSelectLevelCommand(m_mapVM->getSelectLevelCommand());
    m_gameView->setQuitLevelCommand(m_mapVM->getQuitLevelCommand());
    m_gameView->setSelectAircraftCommand(m_mapVM->getSelectAircraftCommand());
    m_gameView->setUseSkillCommand(m_mapVM->getUseSkillCommand());
    m_gameView->setNavigateCommand(m_mapVM->getNavigateCommand());

    // 迭代 6：升级命令
    m_gameView->setUpgradeStatCommand(m_mapVM->getUpgradeStatCommand());

    // ═══════════════════════════════════════════════════════════════
    // ③ 事件绑定 — ViewModel → View（App 只做连接，不监听）
    // ═══════════════════════════════════════════════════════════════

    QObject::connect(m_mapVM, &GameMapVM::propertyChanged,
                     m_gameView, &GameView::onPropertyChanged);

    // ── 持久化绑定（ViewModel 发出保存请求 → App 调 SaveManager）─
    QObject::connect(m_mapVM, &GameMapVM::saveHighScoreRequested,
                     [](int score) {
                         SaveManager().saveHighScore(score);
                         log("AppAgent", "High score saved: " + std::to_string(score));
                     });
    QObject::connect(m_mapVM, &GameMapVM::saveCampaignRequested,
                     [this](int level) {
                         SaveManager().saveCampaignProgress(level);
                         // 仅当新关卡比当前最大解锁更高时才更新，确保不锁住已开放关卡
                         if (level > 1) {
                             int cur = m_mapVM->getMaxUnlockedLevel();
                             if (level > cur) {
                                 m_mapVM->setMaxUnlockedLevel(level);
                                 m_gameView->setLevelSelectMaxUnlocked(level);
                             }
                         }
                         log("AppAgent", "Campaign progress saved: level " + std::to_string(level));
                     });
    QObject::connect(m_mapVM, &GameMapVM::saveUpgradeRequested,
                     [](int starCores, int packedLevels) {
                         SaveManager().saveUpgradeData({starCores, packedLevels});
                         log("AppAgent", "Upgrade data saved");
                     });

    // ── 动态更新（监听 GameState 变化，仅用于战机图片切换）─────
    QObject::connect(m_mapVM, &GameMapVM::propertyChanged,
                     [this](uint32_t id) {
                         if (id == PROP_ID_GAME_STATE && m_mapVM->getGameState() == GameState::Playing) {
                             int type = m_mapVM->getAircraftType();
                             const QPixmap* pix = m_spriteVM->getAircraftPixmap(static_cast<AircraftType>(type));
                             if (pix) m_gameView->setPlayerPixmap(pix);
                         }
                     });

    log("AppAgent", "三绑定建立完成");
    log("AppAgent", "=== 初始化完成 ===");
}

// ═══════════════════════════════════════════════════════════════════
// 运行
// ═══════════════════════════════════════════════════════════════════

int AppAgent::run() {
    m_gameView->show();
    log("AppAgent", "进入事件循环");
    return qApp->exec();
}
