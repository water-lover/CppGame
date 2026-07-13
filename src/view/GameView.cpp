#include "view/GameView.hpp"
#include "view/GameScene.hpp"
#include "view/StartScreen.hpp"
#include "view/ModeSelectScreen.hpp"
#include "view/GameOverScreen.hpp"
#include "view/PauseOverlay.hpp"
#include "view/BossHealthBar.hpp"
#include "view/LevelSelectScreen.hpp"
#include "view/AircraftSelectScreen.hpp"
#include "view/UpgradeScreen.hpp"
#include "view/LevelCompleteScreen.hpp"
#include "view/SplashScreen.hpp"

#include <QGraphicsView>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QResizeEvent>

// ═══════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════

GameView::GameView(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("雷霆战机 — Thunder Fighter"));
    resize(SCREEN_WIDTH, SCREEN_HEIGHT);
    setMinimumSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);
    // 禁用输入法，防止中文输入法拦截 S / Space 等按键
    setAttribute(Qt::WA_InputMethodEnabled, false);

    // ── 页面栈 ───────────────────────────────────────────────────
    m_pageStack = new QStackedWidget(this);
    // 用 layout 管理尺寸，使 QStackedWidget 随 GameView 等比缩放
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_pageStack);

    m_startScreen = new StartScreen(this);
    m_pageStack->addWidget(m_startScreen);  // 0

    m_modeSelectScreen = new ModeSelectScreen(this);
    m_pageStack->addWidget(m_modeSelectScreen);  // 1

    m_levelSelectScreen = new LevelSelectScreen(this);
    m_pageStack->addWidget(m_levelSelectScreen);  // 2

    m_aircraftSelectScreen = new AircraftSelectScreen(this);
    m_pageStack->addWidget(m_aircraftSelectScreen);  // 3

    m_gamePage = new QWidget(this);
    m_gamePage->setFocusPolicy(Qt::NoFocus);  // 防止抢焦点
    {
        QVBoxLayout* layout = new QVBoxLayout(m_gamePage);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        m_scene = new GameScene(this);
        m_graphicsView = new QGraphicsView(m_scene, m_gamePage);
        m_graphicsView->setFrameStyle(QFrame::NoFrame);
        m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
        m_graphicsView->setFocusPolicy(Qt::NoFocus);
        m_graphicsView->setBackgroundBrush(QColor(10, 10, 30));
        // 关键：viewport 默认 StrongFocus，点击画面会抢走焦点导致 S/Space 失效
        m_graphicsView->viewport()->setFocusPolicy(Qt::NoFocus);
        // 禁用 viewport 输入法，双重保险
        m_graphicsView->viewport()->setAttribute(Qt::WA_InputMethodEnabled, false);
        m_bossHealthBar = new BossHealthBar(m_gamePage);
        m_bossHealthBar->setVisible(false);
        m_bossHealthBar->raise();
        layout->addWidget(m_graphicsView);
    }
    m_pageStack->addWidget(m_gamePage);  // 4

    m_gameOverScreen = new GameOverScreen(this);
    m_gameOverScreen->setGeometry(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    m_pageStack->addWidget(m_gameOverScreen);  // 5

    m_pauseOverlay = new PauseOverlay(this);
    m_pauseOverlay->setGeometry(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    m_pageStack->addWidget(m_pauseOverlay);  // 6

    // ── 升级界面（页面 7） ─────────────────────────────────────
    m_upgradeScreen = new UpgradeScreen(this);
    m_pageStack->addWidget(m_upgradeScreen);  // 7
    // 如命令已在 setUpgradeStatCommand 中提前注入，在此转发
    if (m_upgradeStatCommand) {
        auto cpy = m_upgradeStatCommand;
        m_upgradeScreen->setUpgradeStatCommand(std::move(cpy));
    }

    // ── 页面 8: 胜利结算界面 ──────────────────────────────────
    m_levelCompleteScreen = new LevelCompleteScreen(this);
    m_pageStack->addWidget(m_levelCompleteScreen);  // 8
    connect(m_levelCompleteScreen, &LevelCompleteScreen::nextLevelClicked, [this]() {
        if (m_startLevelCommand && m_pCurrentLevel) m_startLevelCommand(*m_pCurrentLevel + 1);
    });
    connect(m_levelCompleteScreen, &LevelCompleteScreen::backToMenuClicked, [this]() {
        if (m_navigateCommand) m_navigateCommand(0);
    });

    // ── 页面 9: 加载过渡画面 ──────────────────────────────────
    m_splashScreen = new SplashScreen(this);
    m_pageStack->addWidget(m_splashScreen);  // 9

    m_pageStack->setCurrentIndex(0);

    // ── 帧循环 ────────────────────────────────────────────────────
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameView::tick);

    // ── 信号连接 ──────────────────────────────────────────────────
    connect(m_startScreen, &StartScreen::startClicked, [this]() {
        m_pageStack->setCurrentIndex(1);
    });

    connect(m_modeSelectScreen, &ModeSelectScreen::modeSelected, [this](int mode) {
        if (mode == 0)
            m_pageStack->setCurrentIndex(2);  // Campaign → LevelSelect
        else
            m_pageStack->setCurrentIndex(3);  // Endless → AircraftSelect
        if (m_selectModeCommand) m_selectModeCommand(mode);
    });

    connect(m_levelSelectScreen, &LevelSelectScreen::levelSelected, [this](int levelId) {
        m_pageStack->setCurrentIndex(3);  // AircraftSelect
        if (m_selectLevelCommand) m_selectLevelCommand(levelId);
    });

    connect(m_levelSelectScreen, &LevelSelectScreen::backClicked, [this]() {
        m_pageStack->setCurrentIndex(1);
    });

    connect(m_aircraftSelectScreen, &AircraftSelectScreen::confirmed, [this]() {
        m_pageStack->setCurrentIndex(4);  // GamePage
        if (m_startGameCommand) m_startGameCommand();
    });

    connect(m_gameOverScreen, &GameOverScreen::restartClicked, [this]() {
        if (m_pLevelCleared && *m_pLevelCleared) {
            if (m_navigateCommand)
                m_navigateCommand(static_cast<int>(GameState::LevelSelect));
            else
                m_pageStack->setCurrentIndex(2);
        } else {
            if (m_navigateCommand)
                m_navigateCommand(static_cast<int>(GameState::ModeSelect));
            else
                m_pageStack->setCurrentIndex(1);
        }
    });

    connect(m_pauseOverlay, &PauseOverlay::resumeClicked, [this]() {
        if (m_pauseCommand) m_pauseCommand();
    });

    connect(m_pauseOverlay, &PauseOverlay::quitLevelClicked, [this]() {
        if (m_quitLevelCommand) m_quitLevelCommand();
    });

    // ── 暂停 → 升级 ──────────────────────────────────────────
    connect(m_pauseOverlay, &PauseOverlay::upgradeClicked, [this]() {
        m_pageStack->setCurrentIndex(7);
        // 进入时刷新升级界面数据
        if (m_upgradeScreen && m_pStarCores)
            m_upgradeScreen->setStarCores(*m_pStarCores);
        if (m_pUpgradeFireLevel) {
            m_upgradeScreen->setUpgradeLevel(0, *m_pUpgradeFireLevel);
            m_upgradeScreen->setUpgradeLevel(1, *m_pUpgradeLivesLevel);
            m_upgradeScreen->setUpgradeLevel(2, *m_pUpgradeSpeedLevel);
            m_upgradeScreen->setUpgradeLevel(3, *m_pUpgradeCooldownLevel);
        }
        if (m_navigateCommand)
            m_navigateCommand(static_cast<int>(GameState::Upgrade));
    });

    // ── 升级 → 返回暂停 ──────────────────────────────────────
    connect(m_upgradeScreen, &UpgradeScreen::backClicked, [this]() {
        if (m_navigateCommand)
            m_navigateCommand(static_cast<int>(GameState::Paused));
        else
            m_pageStack->setCurrentIndex(6);
    });
}

GameView::~GameView() = default;

// ═══════════════════════════════════════════════════════════════════
// 属性绑定 setter
// ═══════════════════════════════════════════════════════════════════

void GameView::setMap(const AirMap* map) noexcept { m_pMap = map; if (m_scene) m_scene->setMap(map); }
void GameView::setPlayerPixmap(const QPixmap* p) noexcept { m_pPlayerImg = p; if (m_scene) m_scene->setPlayerPixmap(p); }
void GameView::setEnemySmallPixmap(const QPixmap* p) noexcept { m_pEnemyImg = p; if (m_scene) m_scene->setEnemySmallPixmap(p); }
void GameView::setBulletPixmap(const QPixmap* p) noexcept { m_pBulletImg = p; if (m_scene) m_scene->setBulletPixmap(p); }
void GameView::setBackgroundPixmap(const QPixmap* p) noexcept { m_pBgImg = p; if (m_scene) m_scene->setBackgroundPixmap(p); }
void GameView::setEnemyMediumPixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setEnemyMediumPixmap(p); }
void GameView::setEnemyLargePixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setEnemyLargePixmap(p); }
void GameView::setBossPixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setBossPixmap(p); }
void GameView::setBossPixmap2(const QPixmap* p) noexcept { if (m_scene) m_scene->setBossPixmap2(p); }
void GameView::setBossPixmap3(const QPixmap* p) noexcept { if (m_scene) m_scene->setBossPixmap3(p); }
void GameView::setBossPixmap4(const QPixmap* p) noexcept { if (m_scene) m_scene->setBossPixmap4(p); }
void GameView::setEnemyBulletPixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setEnemyBulletPixmap(p); }
void GameView::setPowerUpHpPixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setPowerUpHpPixmap(p); }
void GameView::setPowerUpFirePixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setPowerUpFirePixmap(p); }
void GameView::setPowerUpShieldPixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setPowerUpShieldPixmap(p); }
void GameView::setPowerUpStarCorePixmap(const QPixmap* p) noexcept { if (m_scene) m_scene->setPowerUpStarCorePixmap(p); }

void GameView::setLevelSelectMaxUnlocked(int level) noexcept {
    if (m_levelSelectScreen) m_levelSelectScreen->setMaxUnlockedLevel(level);
}

// ═══════════════════════════════════════════════════════════════════
// 命令绑定 setter
// ═══════════════════════════════════════════════════════════════════

void GameView::setTickCommand(std::function<void(float)>&& cmd)    { m_tickCommand = std::move(cmd); }
void GameView::setMoveUpCommand(std::function<void(int)>&& cmd)   { m_moveUpCommand = std::move(cmd); }
void GameView::setMoveDownCommand(std::function<void(int)>&& cmd) { m_moveDownCommand = std::move(cmd); }
void GameView::setMoveLeftCommand(std::function<void(int)>&& cmd) { m_moveLeftCommand = std::move(cmd); }
void GameView::setMoveRightCommand(std::function<void(int)>&& cmd){ m_moveRightCommand = std::move(cmd); }
void GameView::setStartGameCommand(std::function<void()>&& cmd)   { m_startGameCommand = std::move(cmd); }
void GameView::setSelectModeCommand(std::function<void(int)>&& cmd){ m_selectModeCommand = std::move(cmd); }
void GameView::setPauseCommand(std::function<void()>&& cmd)       { m_pauseCommand = std::move(cmd); }
void GameView::setStartLevelCommand(std::function<void(int)>&& cmd){ m_startLevelCommand = std::move(cmd); }
void GameView::setSelectLevelCommand(std::function<void(int)>&& cmd){ m_selectLevelCommand = std::move(cmd); }
void GameView::setQuitLevelCommand(std::function<void()>&& cmd)     { m_quitLevelCommand = std::move(cmd); }
void GameView::setSelectAircraftCommand(std::function<void(int)>&& cmd) {
    m_selectAircraftCommand = cmd;
    if (m_aircraftSelectScreen) {
        auto cpy = m_selectAircraftCommand;
        m_aircraftSelectScreen->setSelectAircraftCommand(std::move(cpy));
    }
}
void GameView::setUseSkillCommand(std::function<void()>&& cmd)       { m_useSkillCommand = std::move(cmd); }
void GameView::setNavigateCommand(std::function<void(int)>&& cmd)    { m_navigateCommand = std::move(cmd); }
void GameView::setUpgradeStatCommand(std::function<void(int)>&& cmd) {
    m_upgradeStatCommand = cmd;
    if (m_upgradeScreen) {
        auto cpy = m_upgradeStatCommand;
        m_upgradeScreen->setUpgradeStatCommand(std::move(cpy));
    }
}
void GameView::setStarCoresPtr(const int* p) noexcept {
    m_pStarCores = p;
    if (m_scene) m_scene->setHudStarCores(p);
}
void GameView::setUpgradeFireLevelPtr(const int* p) noexcept { m_pUpgradeFireLevel = p; }
void GameView::setUpgradeLivesLevelPtr(const int* p) noexcept { m_pUpgradeLivesLevel = p; }
void GameView::setUpgradeSpeedLevelPtr(const int* p) noexcept { m_pUpgradeSpeedLevel = p; }
void GameView::setUpgradeCooldownLevelPtr(const int* p) noexcept { m_pUpgradeCooldownLevel = p; }
void GameView::setMaxUnlockedLevelPtr(const int* p) noexcept {
    m_pMaxUnlockedLevel = p;
    if (m_pMaxUnlockedLevel) {
        setLevelSelectMaxUnlocked(*m_pMaxUnlockedLevel);
    }
}

// ═══════════════════════════════════════════════════════════════════
// 帧循环
// ═══════════════════════════════════════════════════════════════════

void GameView::tick() {
    if (m_scene) m_scene->updateParticles(0.016f);
    if (m_tickCommand) m_tickCommand(0.016f);
}

// ═══════════════════════════════════════════════════════════════════
// 键盘事件
// ═══════════════════════════════════════════════════════════════════

void GameView::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_W:      case Qt::Key_Up:    if (m_moveUpCommand)    m_moveUpCommand(1);    break;
    case Qt::Key_S:      case Qt::Key_Down:  if (m_moveDownCommand)  m_moveDownCommand(1);  break;
    case Qt::Key_A:      case Qt::Key_Left:  if (m_moveLeftCommand)  m_moveLeftCommand(1);  break;
    case Qt::Key_D:      case Qt::Key_Right: if (m_moveRightCommand) m_moveRightCommand(1); break;
    case Qt::Key_Space:
        if (m_useSkillCommand) m_useSkillCommand();
        break;
    case Qt::Key_Return: case Qt::Key_Enter:
        if (m_startGameCommand) m_startGameCommand();
        break;
    case Qt::Key_Escape:
        if (m_pauseCommand) m_pauseCommand();
        break;
    case Qt::Key_F11:
        if (isFullScreen()) showNormal();
        else showFullScreen();
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }
}

void GameView::keyReleaseEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_W:      case Qt::Key_Up:    if (m_moveUpCommand)    m_moveUpCommand(0);    break;
    case Qt::Key_S:      case Qt::Key_Down:  if (m_moveDownCommand)  m_moveDownCommand(0);  break;
    case Qt::Key_A:      case Qt::Key_Left:  if (m_moveLeftCommand)  m_moveLeftCommand(0);  break;
    case Qt::Key_D:      case Qt::Key_Right: if (m_moveRightCommand) m_moveRightCommand(0); break;
    default:
        QWidget::keyReleaseEvent(e);
    }
}

// ═══════════════════════════════════════════════════════════════════
// 窗口缩放
// ═══════════════════════════════════════════════════════════════════

void GameView::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    m_pageStack->setGeometry(0, 0, width(), height());
    if (m_graphicsView) {
        m_graphicsView->fitInView(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::KeepAspectRatio);
    }
    if (m_bossHealthBar && m_gamePage) {
        int barH = std::max(20, m_gamePage->height() / 25);
        m_bossHealthBar->setGeometry(0, 0, m_gamePage->width(), barH);
        m_bossHealthBar->raise();
    }
}

// ═══════════════════════════════════════════════════════════════════
// 事件通知处理
// ═══════════════════════════════════════════════════════════════════

void GameView::onPropertyChanged(uint32_t id) {
    switch (id) {
    case PROP_ID_MAP:
        m_scene->update();
        break;

    case PROP_ID_SCORE:
        if (m_pScore) m_gameOverScreen->setScore(*m_pScore);
        m_scene->update();
        break;

    case PROP_ID_LIVES:
        m_scene->update();
        break;

    case PROP_ID_GAME_STATE:
        updatePage();
        if (*m_pGameState == GameState::GameOver && m_pLevelCleared && m_pCurrentLevel) {
            m_gameOverScreen->setLevelCleared(*m_pLevelCleared, *m_pCurrentLevel);
        }
        break;

    case PROP_ID_BOSS_HEALTH:
        if (m_pBossHp && m_pBossMaxHp) {
            m_bossHealthBar->setHp(*m_pBossHp);
            m_bossHealthBar->setMaxHp(*m_pBossMaxHp);
            m_bossHealthBar->setVisible(*m_pBossMaxHp > 0);
        }
        break;

    case PROP_ID_WAVE:
        m_scene->update();
        break;

    case PROP_ID_STAR_CORES:
        if (m_upgradeScreen && m_pStarCores)
            m_upgradeScreen->setStarCores(*m_pStarCores);
        break;

    case PROP_ID_WEAPON_LEVEL:
        m_scene->update();
        break;

    case PROP_ID_MAX_UNLOCKED_LEVEL:
        if (m_pMaxUnlockedLevel)
            setLevelSelectMaxUnlocked(*m_pMaxUnlockedLevel);
        break;

    case PROP_ID_UPGRADE_LEVELS:
        if (m_upgradeScreen) {
            if (m_pUpgradeFireLevel)
                m_upgradeScreen->setUpgradeLevel(0, *m_pUpgradeFireLevel);
            if (m_pUpgradeLivesLevel)
                m_upgradeScreen->setUpgradeLevel(1, *m_pUpgradeLivesLevel);
            if (m_pUpgradeSpeedLevel)
                m_upgradeScreen->setUpgradeLevel(2, *m_pUpgradeSpeedLevel);
            if (m_pUpgradeCooldownLevel)
                m_upgradeScreen->setUpgradeLevel(3, *m_pUpgradeCooldownLevel);
        }
        break;

    default:
        break;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 界面切换
// ═══════════════════════════════════════════════════════════════════

void GameView::updatePage() {
    if (!m_pGameState) return;
    switch (*m_pGameState) {
    case GameState::Menu:        m_pageStack->setCurrentIndex(0); m_timer->stop(); break;
    case GameState::ModeSelect:  m_pageStack->setCurrentIndex(1); m_timer->stop(); break;
    case GameState::LevelSelect: m_pageStack->setCurrentIndex(2); m_timer->stop(); break;
    case GameState::AircraftSelect: m_pageStack->setCurrentIndex(3); m_timer->stop(); break;
    case GameState::Playing:     m_pageStack->setCurrentIndex(4); m_timer->start(16); setFocus();
        if (m_graphicsView)
            m_graphicsView->fitInView(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::KeepAspectRatio);
        if (m_bossHealthBar && m_gamePage) {
            int barH = std::max(20, m_gamePage->height() / 25);
            m_bossHealthBar->setGeometry(0, 0, m_gamePage->width(), barH);
            m_bossHealthBar->raise();
        }
        break;
    case GameState::Paused:      m_pageStack->setCurrentIndex(6); m_timer->stop(); break;
    case GameState::Upgrade:     m_pageStack->setCurrentIndex(7); m_timer->stop(); break;
    case GameState::GameOver:    m_pageStack->setCurrentIndex(5); m_timer->stop();
        if (m_pScore) m_gameOverScreen->setScore(*m_pScore);
        if (m_pHighScore) m_gameOverScreen->setHighScore(*m_pHighScore);
        if (m_pLevelCleared && m_pCurrentLevel && *m_pLevelCleared)
            m_gameOverScreen->setLevelCleared(true, *m_pCurrentLevel);
        break;
    case GameState::LevelComplete:  // 安全兜底
        m_pageStack->setCurrentIndex(5); m_timer->stop(); break;
    }
}
