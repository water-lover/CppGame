# 迭代 4 — 关卡选关 + 7 关独立设计：Agent 指令（重写版）

> **整体目标**：修复迭代 4 当前实现的所有问题，实现严格的 7 关独立设计
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 第 4.4 节和 `docs/planning/RULES.md`
>
> **⚠️ 问题清单（本次必须修复）**：
> 1. ❌ 每个关卡内容和体验完全一样 → **需要严格的每关独立配置**
> 2. ❌ 游戏中无法退出关卡 → **需要"退出关卡"功能**
> 3. ❌ 暂停功能不完整 → **暂停中应可选"继续/退出关卡"**
> 4. ❌ 关卡选择界面缩放时背景跑到左上角 → **修复 paintEvent 使用 widget 坐标**
> 5. ❌ 波次数量和 BOSS 规则不严格 → **每关固定 3 wave + 第 2 关起 BOSS**

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

**无需修改**（Types.hpp 中 `GameState::LevelSelect` 已在上一轮添加）。

---

## ② ViewModel Agent（主力）

**目录**：`src/viewmodel/` + `include/viewmodel/`

### WaveManager.hpp/cpp — 重写关卡配置

**问题**：目前 `LEVEL_TABLE` 的 waveCount 每个关不一样（3~5），敌人类型分配也是模糊的区间判断。需要严格对齐 DESIGN_PLAN。

#### 更改 1：严格 7 关配置表

```cpp
// ── 7 关关卡配置表（严格对齐 DESIGN_PLAN） ──
// 每关固定 3 波小怪 + （第2关起）BOSS
const WaveConfig LEVEL_TABLE[7] = {
    /* levelId, waveCount, spawnInterval, enemySpeed, hpBonus, hasBoss, bossId */
    { 1, 3, 1.5f, 1.0f, 0, false, 1 },  // 第1关: 初入战场 — 🎯 无BOSS！
    { 2, 3, 1.4f, 1.1f, 0, true,  1 },  // 第2关: 空中走廊 — 中型BOSS(200HP)
    { 3, 3, 1.3f, 1.2f, 1, true,  2 },  // 第3关: 雷云风暴 — 中型BOSS(250HP)
    { 4, 3, 1.2f, 1.3f, 1, true,  2 },  // 第4关: 敌军要塞 — 重型BOSS(350HP)
    { 5, 3, 1.1f, 1.4f, 2, true,  3 },  // 第5关: 暗夜突袭 — 重型BOSS(400HP)
    { 6, 3, 1.0f, 1.5f, 3, true,  3 },  // 第6关: 火力封锁 — 装甲BOSS(500HP)
    { 7, 3, 0.9f, 1.8f, 5, true,  4 },  // 第7关: 最终决战 — 装甲BOSS(600HP)
};
```

**关键点**：
- 所有关的 `waveCount = 3`（固定 3 波小怪）
- 第 1 关 `hasBoss = false`
- BOSS ID 映射：1→中型BOSS(200HP)、2→中型BOSS(250HP)、3→重型BOSS(350/400HP)、4→装甲BOSS(500/600HP)
- HP 值在 Boss 构造函数中根据 bossId 决定

#### 更改 2：严格敌人类型分布（替换 `getEnemyTypeForWave`）

```cpp
std::pair<EnemyType, int> WaveManager::getEnemyTypeForWave(int level, int waveIdx, int spawnIdx) const {
    // ⚠️ 每关严格按 DESIGN_PLAN 设计
    switch (level) {
    case 1:  // 第1关: 初入战场 — 只有小型机，让玩家适应操作
        return { EnemyType::Small, 1 };

    case 2:  // 第2关: 空中走廊 — 小型+中型
        if (spawnIdx % 3 == 0)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };

    case 3:  // 第3关: 雷云风暴 — 中型为主，敌机密度提升
        if (spawnIdx % 2 == 0)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };

    case 4:  // 第4关: 敌军要塞 — 中型+大型，出现大型机
        if (spawnIdx % 3 == 0)
            return { EnemyType::Large, 1 };
        else if (spawnIdx % 3 == 1)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };

    case 5:  // 第5关: 暗夜突袭 — 大量小型+精英，精英机追踪玩家
        if (spawnIdx % 4 == 0)
            return { EnemyType::Elite, 1 };
        else
            return { EnemyType::Small, 2 };

    case 6:  // 第6关: 火力封锁 — 全类型高密度混编
        {
            int r = spawnIdx % 4;
            if (r == 0)      return { EnemyType::Elite, 1 };
            else if (r == 1) return { EnemyType::Large, 1 };
            else if (r == 2) return { EnemyType::Medium, 2 };
            else             return { EnemyType::Small, 2 };
        }

    case 7:  // 第7关: 最终决战 — 精英+大型为主，最强配置
        {
            int r = spawnIdx % 5;
            if (r == 0)      return { EnemyType::Elite, 2 };
            else if (r == 1) return { EnemyType::Large, 2 };
            else if (r == 2) return { EnemyType::Medium, 2 };
            else             return { EnemyType::Small, 2 };
        }

    default:  // 无尽模式用第7关配置
        {
            int r = spawnIdx % 5;
            if (r == 0)      return { EnemyType::Elite, 1 };
            else if (r == 1) return { EnemyType::Large, 1 };
            else if (r == 2) return { EnemyType::Medium, 1 };
            else             return { EnemyType::Small, 1 };
        }
    }
}
```

#### 更改 3：Boss 构造函数增强（Boss.hpp/cpp）

**问题**：BOSS 血量需要通过关卡配置决定，不是所有 BOSS 都一样。

**方案**：Boss 构造函数直接接受 `maxHp` 参数，由 WaveManager 在 `spawnBoss()` 时从 LEVEL_TABLE 传入。

```cpp
// Boss.hpp — 构造函数接受 maxHp
class Boss : public Enemy {
public:
    Boss(float x, float y, float speed, int maxHp);
    int getMaxHp() const { return m_maxHp; }
    // getActorType() 返回 ActorType::Boss
private:
    int m_maxHp;
};

// Boss.cpp
Boss::Boss(float x, float y, float speed, int maxHp)
    : Enemy(x, y, speed)
    , m_maxHp(maxHp)
{
    hp_ = maxHp;
    size_ = (maxHp <= 250) ? 0.10f :    // 中型BOSS
            (maxHp <= 400) ? 0.12f :    // 重型BOSS
                             0.15f;     // 装甲BOSS
}
```

**WaveManager::spawnBoss() 修改**：从 LEVEL_TABLE 获取 BOSS HP。

```cpp
void WaveManager::spawnBoss(std::vector<std::unique_ptr<Enemy>>& enemies) {
    const auto& cfg = getConfigForLevel(currentLevel_);
    // 从配置表确定 BOSS HP
    int bossHp = 0;
    switch (cfg.levelId) {   // 严格对齐 DESIGN_PLAN
    case 2:  bossHp = 200;  break;
    case 3:  bossHp = 250;  break;
    case 4:  bossHp = 350;  break;
    case 5:  bossHp = 400;  break;
    case 6:  bossHp = 500;  break;
    case 7:  bossHp = 600;  break;
    default: bossHp = 200;  break;  // 无尽模式默认
    }
    enemies.push_back(std::make_unique<Boss>(0.5f, -0.2f, 0.15f, bossHp));
}
```

或者更简洁——在 LEVEL_TABLE 中增加 `bossHp` 字段，直接存储：

```cpp
// WaveManager.hpp — WaveConfig 新增 bossHp 字段
struct WaveConfig {
    int   levelId;
    int   waveCount;
    float spawnInterval;
    float enemySpeed;
    int   enemyHpBonus;
    bool  hasBoss;
    int   bossId;
    int   bossHp;        // ← 新增：BOSS 血量
};

// WaveManager.cpp — LEVEL_TABLE 增加 bossHp
const WaveConfig LEVEL_TABLE[7] = {
    { 1, 3, 1.5f, 1.0f, 0, false, 1, 0   },  // 无BOSS
    { 2, 3, 1.4f, 1.1f, 0, true,  1, 200 },  // 中型BOSS 200HP
    { 3, 3, 1.3f, 1.2f, 1, true,  2, 250 },  // 中型BOSS 250HP
    { 4, 3, 1.2f, 1.3f, 1, true,  2, 350 },  // 重型BOSS 350HP
    { 5, 3, 1.1f, 1.4f, 2, true,  3, 400 },  // 重型BOSS 400HP
    { 6, 3, 1.0f, 1.5f, 3, true,  3, 500 },  // 装甲BOSS 500HP
    { 7, 3, 0.9f, 1.8f, 5, true,  4, 600 },  // 装甲BOSS 600HP
};

// spawnBoss 直接取 bossHp
void WaveManager::spawnBoss(std::vector<std::unique_ptr<Enemy>>& enemies) {
    const auto& cfg = getConfigForLevel(currentLevel_);
    enemies.push_back(std::make_unique<Boss>(0.5f, -0.2f, 0.15f, cfg.bossHp));
}
```
**推荐使用第二种方案**（LEVEL_TABLE 加 bossHp 字段），更直观不易出错。

### GameMapVM.hpp/cpp — 新增退出关卡功能

#### 更改 1：新增 `quitLevel` 命令

```cpp
// GameMapVM.hpp — 新增
std::function<void()> getQuitLevelCommand();

// GameMapVM.cpp
std::function<void()> GameMapVM::getQuitLevelCommand() {
    return [this]() { quitLevelImpl(); };
}

void GameMapVM::quitLevelImpl() {
    // 退出关卡 → 回到关卡选择（闯关模式）或模式选择（无尽模式）
    if (m_mode == GameMode::Campaign) {
        m_state = GameState::LevelSelect;
    } else {
        m_state = GameState::ModeSelect;
    }
    // 重置所有游戏状态
    m_enemies.clear();
    m_bullets.clear();
    m_player.reset();
    m_elapsed = 0.0f;
    fireChange(PROP_ID_GAME_STATE);
    fireChange(PROP_ID_MAP);
    log("GameMapVM", "Quit level — back to select");
}
```

#### 更改 2：`startLevelImpl` 确保 WaveManager 正确配置

```cpp
void GameMapVM::startLevelImpl(int levelId) {
    if (levelId < 1 || levelId > 7) return;
    m_currentLevel = levelId;
    // ⚠️ 重置 WaveManager 时传入正确的关卡 ID
    // 不要调用 startGameImpl() 全程重置，而是：
    m_state = GameState::Playing;
    m_player.reset();
    m_enemies.clear();
    m_bullets.clear();
    m_scoreMgr.reset();
    m_elapsed     = 0.0f;
    m_lastScore   = 0;
    m_lastLives   = m_player.getLives();
    m_lastGameOver = false;
    m_lastSkillCD  = 0.0f;
    m_lastWeaponLv = m_player.getWeaponLevel();
    m_flameStormTimer = 0.0f;
    m_isDashing = false;
    m_dashTimer = 0.0f;

    // ⚠️ 用选定的关卡 ID 初始化波次管理器
    m_waveMgr.reset(m_currentLevel);
    m_waveMgr.setEndless(false);  // 闯关模式
    m_powerUpMgr.reset();
    m_skill.init(m_player.getAircraftType());

    syncMap();
    fireChange(PROP_ID_MAP);
    fireChange(PROP_ID_SCORE);
    fireChange(PROP_ID_LIVES);
    fireChange(PROP_ID_GAME_STATE);
    fireChange(PROP_ID_SKILL_COOLDOWN);
    fireChange(PROP_ID_WEAPON_LEVEL);
    fireChange(PROP_ID_WAVE);
    fireChange(PROP_ID_BOSS_HEALTH);

    log("GameMapVM", "Level " + std::to_string(levelId) + " started");
}
```

#### 更改 3：区分"通关胜利" vs "死亡失败"

**问题**：通关第 7 关和死亡都进入 GameOver，无法区分。需要 `isLevelCleared()` getter 和 GameOverScreen 显示不同文字。

**GameMapVM.hpp 新增**：
```cpp
// 新增公开 getter
bool  isLevelCleared() const noexcept { return m_levelCleared; }

// 新增私有成员
bool  m_levelCleared  = false;  // true=通关胜利, false=死亡失败
```

**tickImpl() 关卡完成分支（含胜利标记）**：
```cpp
if (m_waveMgr.isLevelComplete(m_enemies)) {
    int nextLevel = m_currentLevel + 1;
    if (nextLevel > 7) {
        log("GameMapVM", "🎉 All 7 levels cleared! Victory!");
        m_state = GameState::GameOver;
        m_levelCleared = true;
        // 更新最高分
        if (m_scoreMgr.getScore() > m_scoreMgr.getHighScore()) {
            m_scoreMgr.setHighScore(m_scoreMgr.getScore());
        }
        fireChange(PROP_ID_GAME_STATE);
        return;
    }
    m_waveMgr.reset(nextLevel);
    m_currentLevel = nextLevel;
}
```

**玩家死亡分支（checkCollisions 中）**：
```cpp
// 玩家死亡时一定要清除胜利标记
if (m_player.isDead()) {
    m_state = GameState::GameOver;
    m_levelCleared = false;  // ← 死亡失败
    // ...
}
```

**GameOverScreen.hpp/cpp 新增通关/失败区分**：
```cpp
// GameOverScreen.hpp
void setLevelCleared(bool cleared) noexcept;

// GameOverScreen.cpp
void GameOverScreen::setLevelCleared(bool cleared) {
    if (cleared) {
        m_titleLabel->setText(QStringLiteral("🎉 全部通关！"));
        m_titleLabel->setStyleSheet("color: gold; font-size: 36px; font-weight: bold;");
    } else {
        m_titleLabel->setText(QStringLiteral("游戏结束"));
        m_titleLabel->setStyleSheet("color: red; font-size: 36px; font-weight: bold;");
    }
}
```

**AppAgent 桥接 + GameView 传递通关标记**：
```cpp
// AppAgent.hpp — 新增桥接
bool  m_bridgeLevelCleared = false;

// AppAgent.cpp — onViewModelChanged 中
case PROP_ID_GAME_STATE:
    m_bridgeState = m_mapVM->getGameState();
    if (m_bridgeState == GameState::GameOver) {
        m_bridgeLevelCleared = m_mapVM->isLevelCleared();
    }
    break;

// GameView.cpp — onPropertyChanged 中
case PROP_ID_GAME_STATE:
    updatePage();
    if (*m_pGameState == GameState::GameOver && m_pLevelCleared) {
        m_gameOverScreen->setLevelCleared(*m_pLevelCleared);
    }
    break;
```

---

## ③ View Agent（主力）

**目录**：`src/view/` + `include/view/`

### LevelSelectScreen.hpp/cpp — 修复缩放问题

**⚠️ 核心问题**：`paintEvent` 使用了 `SCREEN_WIDTH`/`SCREEN_HEIGHT` 硬编码坐标，窗口缩放时背景和标题位置不正确。

#### 修复：paintEvent 使用 widget 实际尺寸

```cpp
void LevelSelectScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    // ✅ 使用 widget() 实际宽高，而不是硬编码
    float w = width();
    float h = height();

    // 背景填满整个 widget
    painter.fillRect(0, 0, static_cast<int>(w), static_cast<int>(h), QColor(15, 15, 40));

    // 标题居中，基于 widget 宽度
    painter.setPen(QColor(255, 215, 0));
    QFont titleFont(QStringLiteral("Microsoft YaHei"), static_cast<int>(w * 0.04f), QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRectF(0, h * 0.05f, w, h * 0.08f),
                     Qt::AlignCenter, QStringLiteral("选 择 关 卡"));
}
```

**同时修复按钮尺寸为百分比**：

```cpp
QPushButton* LevelSelectScreen::createLevelButton(const LevelInfo& info, bool unlocked) {
    auto* btn = new QPushButton(this);
    // ✅ 使用百分比尺寸（相对于父 widget）
    // 在 setupUI 中通过布局控制，按钮本身不设死尺寸
    btn->setMinimumHeight(100);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 文字和样式不变
    // ...
}
```

建议在 `setupUI()` 中使用布局来控制按钮大小，而不是 `setFixedSize(200, 140)`。

### PauseOverlay.hpp/cpp — 新增"退出关卡"按钮

**⚠️ 问题**：暂停界面只有"继续游戏"，玩家无法退出关卡回到选关界面。

```cpp
// PauseOverlay.hpp — 新增信号
signals:
    void resumeClicked();
    void quitLevelClicked();   // ← 新增：退出关卡

// PauseOverlay.cpp — 布局中新增按钮
// 在"继续游戏"按钮下方添加
auto* quitBtn = new QPushButton(QStringLiteral("退出关卡"), this);
quitBtn->setStyleSheet(/* 类似 continue 按钮但颜色不同 */);
connect(quitBtn, &QPushButton::clicked, this, &PauseOverlay::quitLevelClicked);
```

### GameView.hpp/cpp — 暂停和退出流程

#### 更改 1：新增命令绑定和信号连接

```cpp
// GameView.hpp — 新增
void setQuitLevelCommand(std::function<void()>&& cmd);
// ...
std::function<void()> m_quitLevelCommand;

// GameView.cpp — 暂停界面"退出关卡"连线
connect(m_pauseOverlay, &PauseOverlay::quitLevelClicked, [this]() {
    if (m_quitLevelCommand) m_quitLevelCommand();
});
```

#### 更改 2：暂停流程修正

确保 ESC 按键正确触发暂停/继续：

```cpp
// GameView.cpp keyPressEvent 中暂停逻辑不变
// 但需要确保 m_pauseCommand 在 Playing 和 Paused 状态间切换
```

#### 更改 3：GameView 页面索引修正

```cpp
// 页面 index（严格对齐）：
//   0: StartScreen
//   1: ModeSelectScreen
//   2: LevelSelectScreen
//   3: GamePage（含 QGraphicsView + BOSS血条）
//   4: GameOverScreen
//   5: PauseOverlay

// ✅ updatePage() 索引修正：
case GameState::Menu:        m_pageStack->setCurrentIndex(0); break;
case GameState::ModeSelect:  m_pageStack->setCurrentIndex(1); break;
case GameState::LevelSelect: m_pageStack->setCurrentIndex(2); break;
case GameState::Playing:     m_pageStack->setCurrentIndex(3); break;
case GameState::GameOver:    m_pageStack->setCurrentIndex(4); break;
case GameState::Paused:      m_pageStack->setCurrentIndex(5); break;
```

### GameScene.cpp — 区分关卡主题（可选增强）

如果时间允许，可以为不同关卡设置不同的背景色调（例如第 4 关"敌军要塞"用更暗的底色），在 `GameScene::drawBackground` 中根据 `m_pCurrentLevel`（新增属性指针）改变背景绘制。

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### SaveManager.hpp/cpp — 检查关卡进度存档

确认 `loadCampaignProgress()` 和 `saveCampaignProgress(int level)` 已实现。如果缺失则补全：

```cpp
// SaveManager.hpp
int  loadCampaignProgress();           // 返回已解锁的最高关卡（1~7）
void saveCampaignProgress(int level);  // 保存解锁进度
```

JSON 格式：
```json
{
    "highScore": 1000,
    "campaignLevel": 3
}
```

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### AppAgent.hpp — 新增桥接变量

```cpp
// AppAgent.hpp
int  m_bridgeMaxUnlockedLevel = 1;
bool m_bridgeLevelCleared = false;  // 新增：是否通关胜利
```

### AppAgent.cpp — 完整接线

```cpp
void AppAgent::init() {
    // ... 原有逻辑保持不变 ...

    // ═══════ 迭代 4 新增 ═══════

    // 1. 从存档读取关卡解锁进度
    {
        SaveManager saveMgr;
        int savedLevel = saveMgr.loadCampaignProgress();
        m_bridgeMaxUnlockedLevel = (savedLevel >= 1 && savedLevel <= 7) ? savedLevel : 1;
        m_mapVM->setMaxUnlockedLevel(m_bridgeMaxUnlockedLevel);
        m_gameView->setLevelSelectMaxUnlocked(m_bridgeMaxUnlockedLevel);
        log("AppAgent", "Max unlocked level: " + std::to_string(m_bridgeMaxUnlockedLevel));
    }

    // 2. 命令绑定：startLevel 和 quitLevel
    m_gameView->setStartLevelCommand(m_mapVM->getStartLevelCommand());
    m_gameView->setQuitLevelCommand(m_mapVM->getQuitLevelCommand());
}

// onViewModelChanged 增强
void AppAgent::onViewModelChanged(uint32_t propertyId) {
    switch (propertyId) {
    // ... 原有 SCORE / LIVES / BOSS_HEALTH / WAVE 分支 ...

    case PROP_ID_GAME_STATE: {
        m_bridgeState = m_mapVM->getGameState();

        if (m_bridgeState == GameState::GameOver) {
            // ... 原有最高分保存逻辑 ...

            // 闯关模式：通关胜利才保存进度
            if (m_mapVM->getGameMode() == GameMode::Campaign && m_mapVM->isLevelCleared()) {
                int clearedLevel = m_mapVM->getCurrentLevel();
                int nextLevel = clearedLevel + 1;
                if (nextLevel <= 7 && nextLevel > m_bridgeMaxUnlockedLevel) {
                    SaveManager saveMgr;
                    saveMgr.saveCampaignProgress(nextLevel);
                    m_bridgeMaxUnlockedLevel = nextLevel;
                    m_gameView->setLevelSelectMaxUnlocked(nextLevel);
                    log("AppAgent", "Level " + std::to_string(clearedLevel) +
                        " cleared! Unlocked level " + std::to_string(nextLevel));
                }
            }
        } else if (m_bridgeState == GameState::LevelSelect) {
            // 从游戏中退出 → 回到关卡选择
            // 注意：退出关卡不保存进度，无需同步解锁等级
        }
        break;
    }

    // ...
    }
}
```

---

## ⑥ Test Agent

**目录**：`tests/`

### test_wave_manager.cpp — 新增严格关卡测试

```cpp
TEST_CASE("Level 1 has no boss and only small enemies", "[wave][level1]") {
    WaveManager wm;
    wm.reset(1);
    CHECK(wm.getConfigForLevel(1).hasBoss == false);
    CHECK(wm.getConfigForLevel(1).waveCount == 3);
    // 验证生成的敌机类型都是 Small
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::mt19937 rng(42);
    wm.update(10.0f, enemies, 0.5f, rng);  // 快速模拟多帧
    for (const auto& e : enemies) {
        CHECK(dynamic_cast<EnemySmall*>(e.get()) != nullptr);
    }
}

TEST_CASE("Level 2 has boss after 3 waves", "[wave][level2]") {
    WaveManager wm;
    wm.reset(2);
    CHECK(wm.getConfigForLevel(2).hasBoss == true);
    CHECK(wm.getConfigForLevel(2).waveCount == 3);
}

TEST_CASE("All campaign levels have exactly 3 waves", "[wave]") {
    for (int level = 1; level <= 7; ++level) {
        WaveManager wm;
        wm.reset(level);
        CHECK(wm.getConfigForLevel(level).waveCount == 3);
    }
}
```

### test_game_map_vm.cpp — 新增退出关卡测试

```cpp
TEST_CASE("Quit level returns to LevelSelect in Campaign mode", "[vm][quit]") {
    GameMapVM vm;
    vm.getSelectModeCommand()(0);  // Campaign
    CHECK(vm.getGameState() == GameState::LevelSelect);
    vm.getStartLevelCommand()(1);
    CHECK(vm.getGameState() == GameState::Playing);
    vm.getQuitLevelCommand()();
    CHECK(vm.getGameState() == GameState::LevelSelect);
}
```

---

## 开发顺序

```
① ViewModel (WaveManager 严格7关配置 + GameMapVM 退出关卡)  ← 主力
     ↓
③ View (LevelSelectScreen 缩放修复 + PauseOverlay 退出按钮 + GameView 连线)
     ↓ (①②③ 可并行)
④ Resource (SaveManager 关卡存档检查补全)
     ↓
⑤ App (组装新命令 + 进度管理)
     ↓
⑥ Test (严格关卡测试 + 退出流程测试)
```

---

## 流程变更总结

```
闯关模式完整流程：
  Start → ModeSelect → LevelSelect → Playing(关卡1~7) → 通关 → GameOver(胜利)
                              ↑                              ↑
                         ESC → Pause → [继续/退出关卡]───┘     │
                                        退出关卡──────────────┘
                                                              
  通关胜利 → 解锁下一关 → 存档 → 显示 GameOver(带"胜利"标记)
  死亡失败 → 不保存进度 → 显示 GameOver(带"失败"标记)

逐关严格设计：
  第1关: 3 wave 小型机
  第2关: 3 wave 小型+中型 → BOSS(中型, 200HP)
  第3关: 3 wave 中型为主 → BOSS(中型, 250HP)
  第4关: 3 wave 中型+大型 → BOSS(重型, 350HP)
  第5关: 3 wave 小型+精英 → BOSS(重型, 400HP)
  第6关: 3 wave 全类型高密度 → BOSS(装甲, 500HP)
  第7关: 3 wave 精英+大型为主 → BOSS(装甲, 600HP)
```
