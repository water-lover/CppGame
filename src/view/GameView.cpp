#include "view/GameView.hpp"
#include "view/GameScene.hpp"
#include "view/HudOverlay.hpp"
#include "view/StartScreen.hpp"
#include "view/GameOverScreen.hpp"

#include <QGraphicsView>
#include <QVBoxLayout>
#include <QKeyEvent>

// ═══════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════

GameView::GameView(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("雷霆战机 — Thunder Fighter"));
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);

    // ── 页面栈 ────────────────────────────────────────────────────
    m_pageStack = new QStackedWidget(this);
    m_pageStack->setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // 页面 0: 开始界面
    m_startScreen = new StartScreen(this);
    m_pageStack->addWidget(m_startScreen);  // index 0

    // 页面 1: 游戏页面（QGraphicsView + HUD 叠加）
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
        m_graphicsView->setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        m_graphicsView->setFocusPolicy(Qt::NoFocus);  // 焦点在 GameView

        // HUD 覆盖层（透明，叠加在 QGraphicsView 上方）
        m_hud = new HudOverlay(m_gamePage);
        m_hud->setGeometry(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        m_hud->raise();  // 保持在最上层
        m_hud->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        layout->addWidget(m_graphicsView);
    }
    m_pageStack->addWidget(m_gamePage);  // index 1

    // 页面 2: 游戏结束界面
    m_gameOverScreen = new GameOverScreen(this);
    m_pageStack->addWidget(m_gameOverScreen);  // index 2

    // 默认显示开始界面
    m_pageStack->setCurrentIndex(0);

    // ── 帧循环（60 FPS，对齐 ex5 MainWindow 的 Fl::add_timeout） ──
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameView::tick);
    // 不 start — 等进入 Playing 状态时由 updatePage() 启动

    // ── 开始界面按钮事件 ──────────────────────────────────────────
    connect(m_startScreen, &StartScreen::startClicked, [this]() {
        if (m_startGameCommand) m_startGameCommand();
    });

    // ── 游戏结束界面按钮事件 ──────────────────────────────────────
    connect(m_gameOverScreen, &GameOverScreen::restartClicked, [this]() {
        if (m_startGameCommand) m_startGameCommand();
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
// 事件通知处理（对齐 ex5 MainWindow::get_notification）
// ═══════════════════════════════════════════════════════════════════

void GameView::onPropertyChanged(uint32_t id) {
    switch (id) {
    case PROP_ID_MAP:
        // 通知场景重绘所有精灵
        m_scene->update();
        break;

    case PROP_ID_SCORE:
        if (m_pScore) {
            m_hud->setScore(*m_pScore);
            m_gameOverScreen->setScore(*m_pScore);
        }
        break;

    case PROP_ID_LIVES:
        if (m_pLives) {
            m_hud->setLives(*m_pLives);
        }
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

    case GameState::Playing:
        m_pageStack->setCurrentIndex(1);
        m_timer->start(16);
        setFocus();  // 焦点给 GameView（它有 StrongFocus，能接收键盘事件）
        break;

    case GameState::Paused:
        // 迭代 1 暂不支持暂停
        break;

    case GameState::GameOver:
        m_pageStack->setCurrentIndex(2);
        m_timer->stop();
        if (m_pScore) m_gameOverScreen->setScore(*m_pScore);
        if (m_pHighScore) m_gameOverScreen->setHighScore(*m_pHighScore);
        break;
    }
}
