# 迭代 7 — 特效/音效/界面美化/日志完善：Agent 指令

> **整体目标**：补齐游戏体验短板——雷霆雷击特效、爆炸粒子、音效系统、界面美化、日志完善
>
> **框架依据**：对比课件 ex5/ex6，MVVM 架构已完整实现（三绑定、属性通知、命令模式），迭代 7 专注**游戏体验层**
>
> **状态标记**：❌ = 待完成

---

## ① View Agent（主力）

**目录**：`src/view/` + `include/view/`

### P0 — 雷霆雷击特效 ❌

`include/view/GameScene.hpp` + `src/view/GameScene.cpp`

当前 `handleThunderStrike()` 只扣血，画面上无任何视觉反馈。

**方案**：在 GameScene 中添加闪电绘制函数。

```cpp
// GameScene.hpp — 新增
void drawThunderStrike(QPainter* painter);
```

在 `drawForeground()` 末尾检测 ThunderStrike 触发标志并绘制落雷效果：

```
在 M 帧内从上到下绘制白色锯齿闪电线条 + 短暂全屏闪白
```

**显示时机**：GameMapVM 发射 `PROP_ID_SKILL_ACTIVE` 时，由 ViewModel 附加一个 `m_thunderActive` 标记。

```cpp
// GameMapVM::handleThunderStrike() — 设置标记
m_thunderActive = true;    // View 通过 const bool* 读取
m_thunderTimer = 0.3f;     // 闪电持续 0.3 秒
```

### P1 — 爆炸粒子特效 ❌

`src/view/GameScene.cpp`

**方案**：使用 QPainter 绘制简化的粒子效果，在敌机/子弹消失的位置生成 5~8 个向不同方向扩散的小光点。

**粒子结构**：
```cpp
struct Particle {
    float x, y;       // 当前位置
    float vx, vy;     // 速度
    float life;       // 剩余生命 (秒)
    QColor color;     // 颜色
};
```

**实现位置**：在 `GameScene::drawForeground()` 中遍历粒子列表并绘制。

**触发时机**：GameMapVM 将已死亡的敌人/子弹位置通过 `const AirMap*` 传递给 View（现有机制），GameScene 在绘制时检测新死亡的实体并生成粒子。

### P2 — 技能冷却进度条 ❌

`src/view/GameScene.cpp`

将当前的纯文本 `"技能冷却 X%"` 替换为半圆形或彩色进度条。

**设计**：在右下角用 `drawPie()` 或 `drawArc()` 绘制技能图标环形进度条。
- 冷却中：灰色填充弧，显示剩余 %
- 准备就绪：高亮色闪烁 + 文字 "Ready"
- 激活中：进度条全满，显示技能名称

### P3 — 界面美化 ❌

对以下界面进行视觉升级：

| 界面 | 美化点 |
|------|--------|
| **StartScreen** | 背景加入动态闪烁星空，标题发光效果 |
| **ModeSelectScreen** | 按钮改为发光边框卡片，悬停高亮 |
| **GameOverScreen** | 添加半透明动画背景，分数放大突出 |
| **PauseOverlay** | 按钮改为圆角渐变，添加阴影效果 |

**整体原则**：参考现有 `UpgradeScreen` 的深空渐变风格，保持统一。

### P4 — 加载/过渡画面 ❌

`include/view/SplashScreen.hpp` + `src/view/SplashScreen.cpp`

在各页面切换时添加简短淡入淡出过渡：
- 关卡切换（LevelSelect → Playing）：显示 "Loading Level X..." 1~2 秒
- 游戏结束（Playing → GameOver）：渐变黑屏 0.5 秒

**设计**：纯黑 + 加载文字 + 进度点动画。

---

## ② ViewModel Agent

**目录**：`src/viewmodel/` + `include/viewmodel/`

### P5 — 日志完善 ❌

为以下文件添加统一的 `log()` 调用：

| 文件 | 需要日志的场景 |
|------|---------------|
| `CollisionSystem.cpp` | 碰撞检测触发时（玩家受伤、敌机死亡），异常碰撞（无匹配） |
| `Enemy.cpp` | 敌机生成、敌机攻击、敌机死亡 |
| `Bullet.cpp` | 子弹生成、超出边界销毁 |
| `Player.cpp` | 生命变化（受伤/加血）、武器等级变化、无敌开始/结束 |
| `SkillSystem.cpp` | 技能激活、冷却开始 |
| `ScoreManager.cpp` | 分数变化、最高分更新 |
| `AssetManager.cpp` | 资源加载成功/失败（替换当前的 `qWarning`） |

**日志格式**：`log("ClassName", "Action: details")`，保持一致风格。

### P6 — 关卡胜利界面 ❌

`include/viewmodel/GameMapVM.hpp` + `src/viewmodel/GameMapVM.cpp`

将目前胜利后直接进入 GameOver 的模式改为：

1. 新状态 `GameState::LevelComplete`（新增）
2. `tickImpl` 中闯关完成时设置 `m_state = GameState::LevelComplete` 而非 `GameState::GameOver`
3. 发射 `PROP_ID_GAME_STATE` 通知

```cpp
// TickImpl 闯关完成处
if (m_mode == GameMode::Campaign && m_waveMgr.isLevelComplete(m_enemies)) {
    m_levelCleared = true;
    if (nextLevel > m_maxUnlockedLevel) {
        m_maxUnlockedLevel = nextLevel;
        fireChange(PROP_ID_MAX_UNLOCKED_LEVEL);
    }
    m_state = GameState::LevelComplete;  // 而非 GameOver
    fireChange(PROP_ID_GAME_STATE);
    return;
}
```

### P7 — 关卡统计信息 ❌

`include/viewmodel/ScoreManager.hpp` + `src/viewmodel/ScoreManager.cpp`

新增统计字段：

```cpp
int  m_enemiesKilled  = 0;  // 击杀敌机数
int  m_shotsFired     = 0;  // 发射子弹数
int  m_shotsHit       = 0;  // 命中数
int  m_bossesKilled   = 0;  // 击杀 BOSS 数
float m_waveReached   = 0;  // 达到的波次
float m_timePlayed    = 0;  // 游玩时间(秒)
```

提供 getter 和 const int* 指针供 View 显示。

### P8 — 技能触发标记 ❌

`include/viewmodel/GameMapVM.hpp` + `src/viewmodel/GameMapVM.cpp`

为雷霆号的雷击特效添加布尔标记：

```cpp
// GameMapVM.hpp — 新增成员
bool  m_thunderActive = false;
float m_thunderTimer  = 0.0f;

const bool* getThunderActivePtr() const noexcept { return &m_thunderActive; }
```

```cpp
// handleThunderStrike() — 添加
m_thunderActive = true;
m_thunderTimer = 0.3f;
```

```cpp
// tickImpl 或 applySkillEffects 中 — 计时归零
if (m_thunderActive) {
    m_thunderTimer -= dt;
    if (m_thunderTimer <= 0.0f) m_thunderActive = false;
}
```

---

## ③ App Agent

**目录**：`src/app/` + `include/app/`

### P9 — 绑定新增指针 ❌

`src/app/AppAgent.cpp`

在 `init()` 中添加新指针的绑定：

```cpp
// 雷击特效标记
m_gameView->setThunderActivePtr(m_mapVM->getThunderActivePtr());

// 关卡统计数据指针
m_gameView->setEnemiesKilledPtr(m_mapVM->getEnemiesKilledPtr());
m_gameView->setShotsFiredPtr(m_mapVM->getShotsFiredPtr());
m_gameView->setShotsHitPtr(m_mapVM->getShotsHitPtr());
m_gameView->setBossesKilledPtr(m_mapVM->getBossesKilledPtr());
m_gameView->setWaveReachedPtr(m_mapVM->getWaveReachedPtr());
m_gameView->setTimePlayedPtr(m_mapVM->getTimePlayedPtr());
```

### P10 — LevelComplete / GameOver 页面切换 ❌

`include/view/GameView.hpp` + `src/view/GameView.cpp`

在 `updatePage()` 中添加 `GameState::LevelComplete` 的处理：

```cpp
case GameState::LevelComplete:
    m_pageStack->setCurrentIndex(5);  // 复用 GameOverScreen 或新建页面
    m_timer->stop();
    // 传入胜利标志和统计数据
    break;
```

如果新建 `LevelCompleteScreen`，则添加到页面栈索引 8（或 5+1）。

---

## ④ 开发顺序

```
① View Agent 主力:
   P1 爆炸粒子  ← 基础特效，其他特效依赖此框架
   ↓
   P0 雷霆雷击  ← 粒子框架搭建后实现闪电特效
   ↓
   P2 冷却进度条 ← UI 美化第一步
   ↓
   P3 界面美化   ← 多个界面的视觉升级
   ↓
   P4 过渡画面   ← 最后做，不影响核心功能

② ViewModel Agent (可与 View 并行):
   P5 日志完善   ← 简单但量大
   ↓
   P6 关卡胜利   ← 新增状态
   ↓
   P7 关卡统计   ← 新增数据
   ↓
   P8 雷击标记   ← 配合 P0 使用

③ App Agent (最后):
   P9 绑定指针
   P10 页面切换
```

## 5. 新增/修改文件清单

### 新增文件

| 文件 | 归属 | 内容 |
|------|------|------|
| `include/view/SplashScreen.hpp` | View | 加载/过渡画面 |
| `src/view/SplashScreen.cpp` | View | 过渡画面实现 |
| `include/view/LevelCompleteScreen.hpp` | View | 胜利结算界面 |
| `src/view/LevelCompleteScreen.cpp` | View | 胜利界面实现 |

### 修改文件

| 文件 | 变更 |
|------|------|
| `include/view/GameScene.hpp` | 新增粒子系统、雷击特效、冷却进度条绘制函数 |
| `src/view/GameScene.cpp` | 实现粒子、雷击、冷却进度条、美化 HUD |
| `include/view/GameView.hpp` | 新增 LevelComplete 页面、各指针 setter |
| `src/view/GameView.cpp` | updatePage 添加 LevelComplete 处理 |
| `include/view/StartScreen.hpp` | 美化接口 |
| `src/view/StartScreen.cpp` | 动态星空背景、发光标题 |
| `include/view/GameOverScreen.hpp` | 新增接受统计数据接口 |
| `src/view/GameOverScreen.cpp` | 美化界面、显示统计信息 |
| `include/viewmodel/GameMapVM.hpp` | 新增雷击标记、GameState::LevelComplete |
| `src/viewmodel/GameMapVM.cpp` | 雷击逻辑、胜利逻辑 |
| `include/viewmodel/ScoreManager.hpp` | 新增统计字段 |
| `src/viewmodel/ScoreManager.cpp` | 字段更新逻辑 |
| `src/viewmodel/CollisionSystem.cpp` | 新增日志 |
| `src/viewmodel/Enemy.cpp` | 新增日志 |
| `src/viewmodel/Bullet.cpp` | 新增日志 |
| `src/viewmodel/Player.cpp` | 新增日志 |
| `src/viewmodel/SkillSystem.cpp` | 新增日志 |
| `src/viewmodel/ScoreManager.cpp` | 新增日志 |
| `src/resource/AssetManager.cpp` | 日志替换 qWarning |
| `include/common/Types.hpp` | 新增 GameState::LevelComplete |
| `src/app/AppAgent.cpp` | 绑定新指针、新页面 |
