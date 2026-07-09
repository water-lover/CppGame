# 迭代 2 — 高画质 + BOSS 战 + 7 关闯关：Agent 指令

> **整体目标**：像素提升 + 屏幕适应 + BOSS战 + 闯关模式
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 和 `docs/planning/RULES.md`

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

### 需要修改的文件

| 文件 | 变更 |
|---|---|
| `include/common/Constants.hpp` | 新增 `FULLSCREEN_TOGGLE_KEY = Qt::Key_F11`，`MIN_WINDOW_WIDTH = 600`，`MIN_WINDOW_HEIGHT = 450` |
| `include/common/PropertyIds.hpp` | 新增 `PROP_ID_MODE_SELECT`（模式选择界面切换）、`PROP_ID_BOSS_HEALTH`（BOSS 血量变化）、`PROP_ID_WAVE`（波次变化）、`PROP_ID_PAUSE`（暂停状态变化） |
| `include/common/Types.hpp` | 新增 `GameMode` 枚举（`Campaign, Endless`）；`GameState` 增加 `ModeSelect, Paused`；`ActorType` 增加 `EnemyMedium, EnemyLarge, Boss, PowerUpHp, PowerUpFire, PowerUpShield` |

---

## ② ViewModel Agent

**目录**：`src/viewmodel/` + `include/viewmodel/`

### 需要创建/修改的文件

| 文件 | 变更 |
|---|---|
| `src/viewmodel/WaveManager.hpp/cpp` | **新建** — 波次管理器：支持 7 关关卡表、每关 3~5 波小怪 + BOSS 波；无尽模式难度递增（每轮敌机速度+10%、生命+15%） |
| `src/viewmodel/PowerUpManager.hpp/cpp` | **新建** — 道具管理器：掉落概率、道具效果（回血+1、火力加强15秒、护盾一次免伤） |
| `src/viewmodel/Enemy.hpp/cpp` | 扩展：新增中型机（左右摆动 AI）、大型机（会发射子弹）；新增 BOSS 类（阶段转换、多种攻击模式）；增加 `EnemyType` 区分 |
| `src/viewmodel/Bullet.hpp/cpp` | 扩展：新增敌方子弹类型（红色圆形弹） |
| `scr/viewmodel/Player.hpp/cpp` | 扩展：新增 `weaponLevel`（1~5，拾取火力加强临时提升）；新增 `shieldTimer`（护盾持续时间） |
| `src/viewmodel/GameMapVM.hpp/cpp` | **核心修改**：集成 WaveManager / PowerUpManager；新增模式选择逻辑；BOSS 战触发；关卡过渡逻辑；新增命令 `getPauseCommand()` / `getSelectModeCommand()` |
| `src/viewmodel/SpiritVM.hpp/cpp` | 扩展：新增中型机、大型机、BOSS、道具等图片 setter/getter |

### GameMapVM 新增内容

```cpp
// 新增命令
std::function<void()>       getPauseCommand();
std::function<void(int)>    getSelectModeCommand();   // 0=闯关 1=无尽

// 新增属性暴露
int getWave() const noexcept;
int getBossHp() const noexcept;
int getBossMaxHp() const noexcept;
GameMode getGameMode() const noexcept;

// tickImpl 中新增
void tickImpl(float dt) {
    // ... 原有逻辑 ...
    m_waveManager.update(dt, m_enemies, m_player);     // 波次管理
    m_powerUpMgr.update(dt, m_enemies, m_bullets);     // 道具掉落
    // BOSS 出现时同步血量
}
```

### BOSS AI 设计

```cpp
enum class BossPhase { Phase1, Phase2, Phase3 };

class Boss : public Enemy {
    BossPhase m_phase = BossPhase::Phase1;
    float m_attackTimer = 0.0f;
    int m_attackPattern = 0;  // 0=单发 1=散射 2=弹幕

    void update(float dt) override {
        // 阶段转换（血量 < 50% / < 25%）
        if (m_hp < m_maxHp * 0.25) m_phase = BossPhase::Phase3;
        else if (m_hp < m_maxHp * 0.5) m_phase = BossPhase::Phase2;

        // 每种阶段有不同移动模式和攻击频率
        switch (m_phase) {
        case BossPhase::Phase1: /* 左右平移 + 2s 一发 */ break;
        case BossPhase::Phase2: /* 加速 + 1.5s 三发散弹 */ break;
        case BossPhase::Phase3: /* 狂乱 + 1s 弹幕 */ break;
        }
    }
};
```

---

## ③ View Agent

**目录**：`src/view/` + `include/view/`

### 核心变更：屏幕适应性

```cpp
// GameView.hpp — 去掉 setFixedSize，改为可缩放
class GameView : public QWidget {
    // ...
protected:
    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        // 保持宽高比缩放场景
        if (m_graphicsView)
            m_graphicsView->fitInView(0, 0, 800, 600, Qt::KeepAspectRatio);
    }
    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_F11) {
            if (isFullScreen()) showNormal();
            else showFullScreen();
        }
        // ... 其他按键 ...
    }
};
```

### 需要创建/修改的文件

| 文件 | 变更 |
|---|---|
| `src/view/GameView.cpp` | 去掉 `setFixedSize`；实现 `resizeEvent` + `fitInView`；F11 全屏；支持窗口拉伸 |
| `src/view/HudOverlay.cpp` | 改为百分比定位（分数左上、生命右上、波次居中上方）；新增 BOSS 血条显示；新增道具图标显示 |
| `src/view/StartScreen.cpp` | 加入"闯关模式"/"无尽模式"选择按钮（或等 ModeSelectScreen 单独实现） |
| `src/view/GameScene.cpp` | 扩展绘制：中型机（红色）、大型机（黑色）、BOSS（特殊大图）、道具（不同颜色图标）；按精灵类型区分大小 |
| `src/view/ModeSelectScreen.hpp/cpp` | **新建** — 模式选择界面：两个大按钮"闯关模式"/"无尽模式" |
| `src/view/BossHealthBar.hpp/cpp` | **新建** — BOSS 血条：场景顶部显示，随 BOSS 血量变化 |
| `src/view/PauseOverlay.hpp/cpp` | **新建** — 暂停覆盖层：半透明遮罩 + "继续"按钮 |
| `src/view/PlayerItem.cpp` | 扩展：支持武器等级显示（不同发射效果） |
| `src/view/EnemyItem.cpp` | 扩展：支持中型机、大型机、BOSS 不同图片和大小 |
| `src/view/BulletItem.cpp` | 扩展：不同颜色区分玩家子弹（蓝色）和敌方子弹（红色） |

### HUD 百分比布局

```cpp
// HudOverlay.cpp — 不再用绝对像素，用百分比
void HudOverlay::paintEvent(QPaintEvent*) {
    float w = width();   // 实际窗口宽
    float h = height();  // 实际窗口高

    // 分数 — 左上角，距左边 2%，距顶边 2%
    painter.drawText(QRectF(w * 0.02, h * 0.02, w * 0.3, h * 0.05), ...);

    // 生命 — 右上角，距右边 2%，距顶边 2%
    painter.drawText(QRectF(w * 0.68, h * 0.02, w * 0.3, h * 0.05), ...);

    // 波次 — 中上方
    painter.drawText(QRectF(w * 0.35, h * 0.02, w * 0.3, h * 0.05), ...);
}
```

### 界面切换（QStackedWidget 新增页面）

```
GameView 的 QStackedWidget：
  index 0: StartScreen
  index 1: ModeSelectScreen     ← 新增
  index 2: GamePage（游戏场景 + HUD）
  index 3: GameOverScreen
  index 4: PauseOverlay         ← 新增
```

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### 需要修改的文件

| 文件 | 变更 |
|---|---|
| `include/resource/AssetManager.hpp` | 无需修改接口，已有 `getImage(key)` 通用方法 |
| `src/resource/AssetManager.cpp` | 无需修改 |
| `src/resource/SaveManager.cpp` | 扩展：新增 `saveCampaignProgress(int level)` / `loadCampaignProgress()`；存档结构增加关卡进度字段 |

### QRC 资源新增

`resources/resources.qrc` 需要新增图片 alias：

```xml
<file alias="enemyMedium">images/PNG/Enemies/enemyBlack1.png</file>
<file alias="enemyLarge">images/PNG/Enemies/enemyBlack5.png</file>
<file alias="bossShip">images/PNG/Sprites/Ships/spaceShips_009.png</file>
<file alias="bossRocket">images/PNG/Sprites/Rockets/spaceRockets_001.png</file>
<file alias="enemyBullet">images/PNG/Lasers/laserRed01.png</file>
<file alias="powerUpHp">images/PNG/Power-ups/pill_red.png</file>
<file alias="powerUpFire">images/PNG/Power-ups/powerupBlue_bolt.png</file>
<file alias="powerUpShield">images/PNG/Power-ups/shield_gold.png</file>
```

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### 需要修改的文件

`src/app/AppAgent.cpp`：

```cpp
void AppAgent::init() {
    // ... 迭代1原有逻辑 ...

    // 新增属性绑定
    m_gameView->setWavePtr(&m_bridgeWave);
    m_gameView->setBossHpPtr(&m_bridgeBossHp);
    m_gameView->setBossMaxHpPtr(&m_bridgeBossMaxHp);

    // 新增命令绑定
    m_gameView->setPauseCommand(m_mapVM->getPauseCommand());
    m_gameView->setSelectModeCommand(m_mapVM->getSelectModeCommand());

    // SpiritVM 注入新图片
    m_spriteVM->setEnemyMediumPixmap(assets.getImage("enemyMedium"));
    m_spriteVM->setEnemyLargePixmap(assets.getImage("enemyLarge"));
    m_spriteVM->setBossPixmap(assets.getImage("bossShip"));
    // ...

    // 桥接变量新增
    m_bridgeWave      = 0;
    m_bridgeBossHp    = 0;
    m_bridgeBossMaxHp = 0;
}
```

---

## ⑥ Test Agent

**目录**：`tests/`

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `tests/test_wave_manager.cpp` | WaveManager 波次生成、关卡表、BOSS 触发测试 |
| `tests/test_power_up.cpp` | PowerUpManager 掉落概率、拾取效果测试 |
| `tests/test_boss.cpp` | BOSS 阶段转换、攻击模式测试 |

### 现有文件扩展

| 文件 | 新增内容 |
|---|---|
| `tests/test_game_map_vm.cpp` | 新增模式选择、暂停恢复、BOSS 战集成测试 |
| `tests/test_collision.cpp` | 新增 BOSS 碰撞、道具碰撞测试 |

---

## 开发顺序

```
① Common (新增枚举/常量/属性ID)
     ↓
② ViewModel (WaveManager → Enemy/Boss扩展 → PowerUpManager → GameMapVM集成)
     ↓
④ Resource (QRC新增 + SaveManager扩展)  ← 可与 ViewModel 并行
     ↓
③ View (屏幕适应 → 新界面 → 新绘制逻辑)
     ↓
⑤ App (组装新增绑定)
     ↓
⑥ Test (全流程覆盖)
```
