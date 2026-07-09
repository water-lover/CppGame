#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/CollisionSystem.hpp"
#include "common/Logger.hpp"
#include <chrono>
#include <algorithm>

GameMapVM::GameMapVM(QObject* parent)
    : QObject(parent)
    , m_rng(std::chrono::steady_clock::now().time_since_epoch().count())
{
}

// ═══════════════════════════════════════════════════════════════════
// 命令 getter — 返回 std::function，由 App 注入给 View
// ═══════════════════════════════════════════════════════════════════

std::function<void()> GameMapVM::getStartGameCommand() {
    return [this]() { startGameImpl(); };
}

std::function<void(int)> GameMapVM::getMoveUpCommand() {
    return [this](int active) { moveUpImpl(active); };
}

std::function<void(int)> GameMapVM::getMoveDownCommand() {
    return [this](int active) { moveDownImpl(active); };
}

std::function<void(int)> GameMapVM::getMoveLeftCommand() {
    return [this](int active) { moveLeftImpl(active); };
}

std::function<void(int)> GameMapVM::getMoveRightCommand() {
    return [this](int active) { moveRightImpl(active); };
}

std::function<void(float)> GameMapVM::getTickCommand() {
    return [this](float dt) { tickImpl(dt); };
}

std::function<void(int)> GameMapVM::getSelectModeCommand() {
    return [this](int mode) { selectModeImpl(mode); };
}

std::function<void()> GameMapVM::getPauseCommand() {
    return [this]() { pauseImpl(); };
}

// ═══════════════════════════════════════════════════════════════════
// 命令实现
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::startGameImpl() {
    m_state = GameState::Playing;
    m_player.reset();
    m_enemies.clear();
    m_bullets.clear();
    m_scoreMgr.reset();
    m_spawnTimer = 0.0f;
    m_elapsed    = 0.0f;
    m_lastScore  = 0;
    m_lastLives  = m_player.getLives();
    m_lastGameOver = false;

    syncMap();

    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_SCORE);
    fireChange(PROP_ID_LIVES);
    fireChange(PROP_ID_GAME_STATE);

    log("GameMapVM", "Game started");
}

void GameMapVM::tickImpl(float dt) {
    if (m_state != GameState::Playing) return;

    m_elapsed += dt;

    // 1. 更新玩家
    m_player.update(dt);

    // 2. 玩家自动射击
    if (m_player.canFire(dt)) {
        Vec2 p = m_player.getPos();
        m_bullets.emplace_back(p.x, p.y - m_player.getSize(),
                                0.0f, -BULLET_SPEED, Bullet::Player);
    }

    // 3. 生成敌机
    m_spawnTimer += dt;
    if (m_spawnTimer >= SPAWN_INTERVAL) {
        m_spawnTimer = 0.0f;
        spawnEnemy();
    }

    // 4. 更新敌机
    for (auto& e : m_enemies) {
        if (!e->isDead()) e->update(dt);
    }

    // 5. 更新子弹
    for (auto& b : m_bullets) b.update(dt);

    // 6. 碰撞检测
    checkCollisions();

    // 7. 清理离屏/死亡实体
    cleanupEntities();

    // 8. 同步到 AirMap
    syncMap();

    // 9. 发出属性变化通知（只在值变化时 emit，减少 View 刷新）
    int curScore = m_scoreMgr.getScore();
    if (curScore != m_lastScore) {
        m_lastScore = curScore;
        fireChange(PROP_ID_SCORE);
    }

    int curLives = m_player.getLives();
    if (curLives != m_lastLives) {
        m_lastLives = curLives;
        fireChange(PROP_ID_LIVES);
    }

    // 地图数据（精灵位置）每帧都变 → 总是通知
    fireChange(PROP_ID_MAP);

    // 游戏结束状态变化
    bool isOver = (m_state == GameState::GameOver);
    if (isOver != m_lastGameOver) {
        m_lastGameOver = isOver;
        if (isOver) {
            // 更新最高分
            if (m_scoreMgr.getScore() > m_scoreMgr.getHighScore()) {
                m_scoreMgr.setHighScore(m_scoreMgr.getScore());
            }
        }
        fireChange(PROP_ID_GAME_STATE);
    }
}

void GameMapVM::moveUpImpl(int active)    { m_player.moveUp(active != 0); }
void GameMapVM::moveDownImpl(int active)  { m_player.moveDown(active != 0); }
void GameMapVM::moveLeftImpl(int active)  { m_player.moveLeft(active != 0); }
void GameMapVM::moveRightImpl(int active) { m_player.moveRight(active != 0); }

void GameMapVM::selectModeImpl(int mode) {
    m_state = GameState::Playing;
    m_player.reset();
    m_enemies.clear();
    m_bullets.clear();
    m_scoreMgr.reset();
    m_spawnTimer = 0.0f;
    m_elapsed    = 0.0f;
    m_lastScore  = 0;
    m_lastLives  = m_player.getLives();
    m_lastGameOver = false;

    syncMap();

    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_SCORE);
    fireChange(PROP_ID_LIVES);
    fireChange(PROP_ID_GAME_STATE);

    log("GameMapVM", "Mode selected: " + std::to_string(mode));
}

void GameMapVM::pauseImpl() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
    }
    fireChange(PROP_ID_GAME_STATE);
}

// ═══════════════════════════════════════════════════════════════════
// 内部工具
// ═══════════════════════════════════════════════════════════════════

void GameMapVM::spawnEnemy() {
    std::uniform_real_distribution<float> distX(0.05f, 0.95f);
    float x = distX(m_rng);
    m_enemies.push_back(std::make_unique<Enemy>(x, -0.1f, ENEMY_SPEED));
}

void GameMapVM::checkCollisions() {
    // 子弹 vs 敌机
    auto hitEnemies = CollisionSystem::checkBulletEnemy(m_bullets, m_enemies);
    for (int ei : hitEnemies) {
        m_scoreMgr.addScore(ENEMY_SCORE);
    }

    // 敌机 vs 玩家
    int hitPlayer = CollisionSystem::checkEnemyPlayer(m_enemies, m_player);
    if (hitPlayer >= 0) {
        m_player.takeDamage();
        m_enemies[hitPlayer]->takeDamage();  // 同归于尽
        if (m_player.isDead()) {
            m_state = GameState::GameOver;
            log("GameMapVM", "Player died — Game Over");
        }
    }
}

void GameMapVM::cleanupEntities() {
    auto removeDead = [](auto& container, auto isDead) {
        container.erase(
            std::remove_if(container.begin(), container.end(), isDead),
            container.end());
    };
    removeDead(m_enemies, [](const auto& e) { return e->isDead() || e->isOffScreen(); });
    removeDead(m_bullets, [](const auto& b) { return b.isOffScreen(); });
}

void GameMapVM::syncMap() {
    m_map.clear();

    // 玩家
    {
        Actor playerActor;
        playerActor.type  = ActorType::Player;
        playerActor.x     = m_player.getPos().x;
        playerActor.y     = m_player.getPos().y;
        playerActor.hp    = m_player.getLives();
        playerActor.maxHp = PLAYER_MAX_LIVES;
        m_map.append(playerActor);
    }

    // 敌机
    for (const auto& e : m_enemies) {
        if (!e->isDead()) {
            Actor enemyActor;
            enemyActor.type  = ActorType::EnemySmall;
            enemyActor.x     = e->getPos().x;
            enemyActor.y     = e->getPos().y;
            enemyActor.hp    = e->getHp();
            enemyActor.maxHp = 1;
            m_map.append(enemyActor);
        }
    }

    // 子弹
    for (const auto& b : m_bullets) {
        Actor bulletActor;
        bulletActor.type  = (b.getOwner() == Bullet::Player)
                            ? ActorType::PlayerBullet
                            : ActorType::EnemyBullet;
        bulletActor.x     = b.getPos().x;
        bulletActor.y     = b.getPos().y;
        bulletActor.hp    = 1;
        bulletActor.maxHp = 1;
        m_map.append(bulletActor);
    }
}

void GameMapVM::fireChange(uint32_t id) {
    emit propertyChanged(id);
}
