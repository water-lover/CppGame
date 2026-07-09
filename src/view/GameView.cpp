#include "view/GameView.hpp"
#include "view/GameScene.hpp"
#include "view/HudOverlay.hpp"
#include "view/StartScreen.hpp"
#include "view/ModeSelectScreen.hpp"
#include "view/GameOverScreen.hpp"
#include "view/PauseOverlay.hpp"

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

    // ── 页面栈（随窗口拉伸） ──────────────────────────────────────
    m_pageStack = new QStackedWidget(this);
    m_pageStack->setGeometry(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 页面 0: 开始界面
    m_startScreen = new StartScreen(this);
    m_pageStack->addWidget(m_startScreen);  // index 0

    // 页面 1: 模式选择界面
    m_modeSelectScreen = new ModeSelectScreen(this);
    m_pageStack->addWidget(m_modeSelectScreen);  // index 1

    // 页面 2: 游戏页面（QGraphicsView + HUD 叠加）
    m_gamePage = new QWidget(this);
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
        m_graphicsView->setBackgroundBrush(QColor(10, 10, 30));  // 黑边替代白边

        layout->addWidget(m_graphicsView);
    }
    m_pageStack->addWidget(m_gamePage);  // index 2

    // 页面 3: 游戏结束界面
    m_gameOverScreen = new GameOverScreen(this);
    m_gameOverScreen->setGeometry(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    m_pageStack->addWidget(m_gameOverScreen);  // index 3

    // 页面 4: 暂停覆盖层
    m_pauseOverlay = new PauseOverlay(this);
    m_pauseOverlay->setGeometry(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    m_pageStack->addWidget(m_pauseOverlay);  // index 4

    // 默认显示开始界面
    m_pageStack->setCurrentIndex(0);

    // ── 帧循环（60 FPS） ──────────────────────────────────────────
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameView::tick);
    // 不 start — 等进入 Playing 时由 updatePage() 启动

    // ── 开始界面 → 模式选择 ──────────────────────────────────────
    connect(m_startScreen, &StartScreen::startClicked, [this]() {
        m_pageStack->setCurrentIndex(1);  // 先切到模式选择
    });

    // ── 模式选择 → 开始游戏 ──────────────────────────────────────
    connect(m_modeSelectScreen, &ModeSelectScreen::modeSelected, [this](int mode) {
        if (m_selectModeCommand) m_selectModeCommand(mode);
    });

    // ── 游戏结束 → 再来一局（回到模式选择） ──────────────────────
    connect(m_gameOverScreen, &GameOverScreen::restartClicked, [this]() {
        m_pageStack->setCurrentIndex(1);  // 回到模式选择
    });

    // ── 暂停 → 继续 ───────────────────────────────────────────────
    connect(m_pauseOverlay, &PauseOverlay::resumeClicked, [this]() {
        if (m_pauseCommand) m_pauseCommand();
    });
}

GameView::~GameView() = default;

// ═══════════════════════════════════════════════════════════════════
// 属性绑定 setter
// ═══════════════════════════════════════════════════════════════════

void GameView::setMap(const AirMap* map) noexcept {
    m_pMap = map;
    if (m_scene) m_scene->setMap(map);
}

void GameView::setPlayerPixmap(const QPixmap* p) noexcept {
    m_pPlayerImg = p;
    if (m_scene) m_scene->setPlayerPixmap(p);
}

void GameView::setEnemySmallPixmap(const QPixmap* p) noexcept {
    m_pEnemyImg = p;
    if (m_scene) m_scene->setEnemySmallPixmap(p);
}

void GameView::setBulletPixmap(const QPixmap* p) noexcept {
    m_pBulletImg = p;
    if (m_scene) m_scene->setBulletPixmap(p);
}

void GameView::setBackgroundPixmap(const QPixmap* p) noexcept {
    m_pBgImg = p;
    if (m_scene) m_scene->setBackgroundPixmap(p);
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

// ═══════════════════════════════════════════════════════════════════
// 帧循环
// ═══════════════════════════════════════════════════════════════════

void GameView::tick() {
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
    // QStackedWidget 会自动管理子页面大小，不需要手动循环
    if (m_graphicsView) {
        // 场景按 800x600 比例适应
        m_graphicsView->fitInView(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::KeepAspectRatio);
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
        if (m_pScore) {
            m_gameOverScreen->setScore(*m_pScore);
        }
        m_scene->update();  // HUD 直接在场景中绘制
        break;

    case PROP_ID_LIVES:
        m_scene->update();  // HUD 直接在场景中绘制
        break;

    case PROP_ID_GAME_STATE:
        updatePage();
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
    case GameState::Menu:
        m_pageStack->setCurrentIndex(0);
        m_timer->stop();
        break;

    case GameState::ModeSelect:
        m_pageStack->setCurrentIndex(1);
        m_timer->stop();
        break;

    case GameState::Playing:
        m_pageStack->setCurrentIndex(2);
        m_timer->start(16);
        setFocus();
        break;

    case GameState::Paused:
        m_pageStack->setCurrentIndex(4);
        m_timer->stop();
        break;

    case GameState::GameOver:
        m_pageStack->setCurrentIndex(3);
        m_timer->stop();
        if (m_pScore) m_gameOverScreen->setScore(*m_pScore);
        if (m_pHighScore) m_gameOverScreen->setHighScore(*m_pHighScore);
        break;
    }
}
