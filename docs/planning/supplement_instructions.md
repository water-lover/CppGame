# 迭代文件未覆盖的功能补全记录

> **整体目标**：记录并补全迭代1~7文件中未涉及但在实际代码中已实现的跨层功能，确保每个功能点都有对应的迭代文档规格。
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 和 `docs/planning/RULES.md`
>
> **状态标记**：✅ = 已完成（已在代码中实现） ❌ = 待完成（仍需补充）

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

### C1 — Type/Property/Actor 枚举扩展（已在代码中，补登规格） ✅

| 文件                               | 已有但未在迭代文件中明确规格的内容                                                                                                                                        |
| ---------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `include/common/Types.hpp`       | `GameState::AircraftSelect`(战机选择)、`GameState::Upgrade`(升级界面)、`GameState::LevelComplete`(关卡胜利)                                                         |
| `include/common/Actor.hpp`       | `ActorType::PowerUpStarCore`(星核碎片道具)                                                                                                                              |
| `include/common/PropertyIds.hpp` | `PROP_ID_SKILL_COOLDOWN`、`PROP_ID_WEAPON_LEVEL`、`PROP_ID_STAR_CORES`、`PROP_ID_UPGRADE_LEVELS`、`PROP_ID_MAX_UNLOCKED_LEVEL`（迭代6部分规格，但最终枚举完备） |

### C2 — Common 层瘦身（架构清理，无需改动） ✅

实际代码中 `MathUtils.hpp`/`Geometry.hpp` 位于 `viewmodel/`、`GameConstants.hpp` 位于 `viewmodel/`、`Logger.hpp` 位于 `resource/`，符合 RULES.md 的"Common 只放被两个及以上不同层使用的代码"原则。此项已正确，仅在此记录规格。

**Common 最终文件清单**：

```
include/common/
├── Actor.hpp          ← 精灵数据结构（View+VM 使用）
├── AirMap.hpp         ← 精灵集合容器（View+VM 使用）
├── Types.hpp          ← GameState 跨层通信协议（View+VM 使用）
└── PropertyIds.hpp    ← 属性 ID 枚举（View+VM 使用）

src/common/
└── AirMap.cpp
```

---

## ② ViewModel Agent

**目录**：`src/viewmodel/` + `include/viewmodel/`

### V1 — 战机专属射击模式（已有，补登规格） ✅

**文件**：`GameMapVM.cpp` — `tickImpl()` 中自动射击部分

5 架战机在射击时拥有不同的弹道模式，这超出了 `AircraftStats` 属性模板的范畴（迭代5仅定义了属性数据，未定义射击行为）：

| 战机              | 射击模式                        | 实现位置                     |
| ----------------- | ------------------------------- | ---------------------------- |
| 雷霆号 (Thunder)  | 双弹 + 武器等级≥3 时中间加一弹 | `GameMapVM.cpp:tickImpl()` |
| 烈焰号 (Flame)    | 3 发散弹（左中右）              | `GameMapVM.cpp:tickImpl()` |
| 冰霜号 (Frost)    | 双弹，同雷霆号基础模式          | `GameMapVM.cpp:tickImpl()` |
| 幻影号 (Phantom)  | 单发高速弹（速度×1.2）         | `GameMapVM.cpp:tickImpl()` |
| 堡垒号 (Fortress) | 双发偏移弹（左右微偏）          | `GameMapVM.cpp:tickImpl()` |

**规格化接口**：

```cpp
// GameMapVM.hpp — tickImpl 中根据 AircraftType 分支选择弹道
// 每种战机在 tickImpl 中 3. 玩家自动射击 处有独立的射击逻辑
```

### V2 — 技能无敌机制（已有，补登规格） ✅

**文件**：`Player.hpp/cpp`、`GameMapVM.cpp`

技能激活期间玩家进入无敌状态，免疫所有伤害：

- `Player::setSkillInvincible(bool)` / `isSkillInvincible()` — 存储技能无敌标记
- `GameMapVM::useSkillImpl()` — 释放技能时调用 `m_player.setSkillInvincible(true)`
- `GameMapVM::applySkillEffects()` — 技能结束时（`!m_skill.isActive()`）调用 `m_player.setSkillInvincible(false)`
- `GameMapVM::checkCollisions()` — 玩家受伤前检查 `m_player.isSkillInvincible()` 和护盾

**数据流**：

```
useSkillImpl() → setSkillInvincible(true)
     ↓
applySkillEffects(dt) 每帧检测技能是否结束
     ↓
技能结束 → setSkillInvincible(false)
```

### V3 — 护盾道具保护机制（已有，补登规格） ✅

**文件**：`Player.hpp/cpp`、`GameMapVM.cpp`

道具护盾 `hasShield` 与技能无敌 `skillInvincible` 分开跟踪：

- `Player::hasShield()` — 拾取金色护盾道具后获得
- `Player::getHasShieldPtr()` — 供 View 显示护盾光环
- 碰撞检测中优先消耗护盾再扣血

```cpp
// Player.hpp — 护盾与无敌分开
bool  m_hasShield = false;               // 道具护盾
bool  m_skillInvincible = false;         // 技能无敌

// GameMapVM.cpp checkCollisions() — 敌方子弹击中玩家
if (reflectEnabled) continue;         // 铁壁阵反射
if (m_player.isSkillInvincible()) continue; // 技能无敌
if (m_player.hasShield()) {
    m_player.setShielded(false);      // 消耗护盾
    continue;
}
m_player.takeDamage();                // 实际扣血
```

### V4 — Boss 攻击模式完整实现（已有，补登规格） ✅

**文件**：`Boss.hpp/cpp`

Boss 拥有完整的阶段转换+弹幕系统，但迭代3/4仅定义了接口框架，未覆盖具体弹幕实现：

| 攻击类型     | 函数                                             | 效果                        |
| ------------ | ------------------------------------------------ | --------------------------- |
| 单发瞄准     | `spawnSingleShot()`                            | 一颗子弹指向玩家当前位置    |
| 双发         | `spawnDoubleShot()`                            | 两发子弹呈 V 形向下         |
| 3 发散弹     | `spawnSpreadShot()`                            | 三发子弹呈扇形(-1, 0, +1)   |
| 瞄准弹       | `spawnAimedShot()`                             | 三发瞄准弹（主弹+两侧偏移） |
| 全屏圆形弹幕 | `spawnBarrage()` → `spawnCircularBarrage()` | 12 发子弹 360° 均匀分布    |

**阶段转换规则**：

```
BossPhase::Phase1 (100%-50%) → Phase2 (50%-25%) → Phase3 (25%-0%)
Phase1: 双发攻击, 间隔 2.0s
Phase2: 3 发散弹 + 瞄准弹交替, 间隔 1.5s
Phase3: 全屏圆形弹幕, 间隔 1.2s
```

**工具函数**（供 Boss 和未来敌人使用）：

```cpp
// Boss.hpp
void spawnCircularBarrage(float cx, float cy, int count, float speed, std::vector<Bullet>& bullets);
void spawnSpiralBarrage(float cx, float cy, int count, float speed, float baseAngle, std::vector<Bullet>& bullets);
```

### V5 — Boss 配置表完整 6 种（已有，补登规格） ✅

**文件**：`Boss.cpp` — `k_BossData[]`

| bossId | maxHp | speed | size | 命名     | 对应关卡 |
| ------ | ----- | ----- | ---- | -------- | -------- |
| 1      | 200   | 0.06  | 0.10 | 中型BOSS | 第2关    |
| 5      | 250   | 0.05  | 0.10 | 中型BOSS | 第3关    |
| 2      | 350   | 0.05  | 0.12 | 重型BOSS | 第4关    |
| 6      | 400   | 0.04  | 0.13 | 重型BOSS | 第5关    |
| 3      | 500   | 0.04  | 0.14 | 装甲BOSS | 第6关    |
| 4      | 600   | 0.03  | 0.15 | 装甲BOSS | 第7关    |

### V6 — 三种持续型技能效果（已有，补登规格） ✅

**文件**：`GameMapVM.cpp` — `handleFlameStorm/d/handleTimeDash/handleIronWall`

| 技能                   | 对应战机 | 效果                        | 实现                                              |
| ---------------------- | -------- | --------------------------- | ------------------------------------------------- |
| 火焰风暴`FlameStorm` | 烈焰号   | 持续发射 5 发散弹扇形火焰   | `handleFlameStorm(dt)` 每 0.1s 5 发             |
| 时空闪避`TimeDash`   | 幻影号   | 冲刺+攻击前方敌人+持续弹幕  | `handleTimeDash(dt)` 0.5s 冲刺+前方弹幕         |
| 铁壁阵`IronWall`     | 堡垒号   | 8 方向反击弹幕+反射敌方子弹 | `handleIronWall(dt)` 每 0.15s 8 发+碰撞检测反射 |

### V7 — 无尽模式关卡循环+难度递增（已有，补登规格） ✅

**文件**：`WaveManager.cpp`

无尽模式打完 7 关后回到第 1 关，`endlessLoop_` 递增：

```
Wave 1 (loop=2): 难度 = 闯关的 100% (与闯关一致)
Wave 2 (loop=3): 难度 = 闯关的 150% (100% + 0.5×)
Wave n (loop=n): 难度 = 100% + (n-2) × 50%
```

**⚠️ 当前实现**：loop=1 时难度为 100% 而非迭代6规定的 50%（easy 起始）。已在 iter4/WaveManager 中实现。

波次显示格式：无尽模式下显示 `"2-3"` 表示第 2 轮第 3 关。

### V8 — 5 战机独立升级数据打包（已有，补登规格） ✅

**文件**：`UpgradeManager.hpp/cpp`

`UpgradeManager` 支持 5 架战机独立的升级等级存储（迭代6仅定义了单个战机的 4 项升级，未覆盖多战机）：

```cpp
// UpgradeManager.hpp — 每架战机 4 项属性，打包存储
void packAllLevels(int out[5]) const;     // 5 架战机 × 4 项各 4bit = 16bit/战机
void setAllLevelsFromArray(const int data[5]);
void setCurrentAircraft(int type);
```

打包格式：每架战机 `levelsPacked` 用 16bit 编码 4 项等级（各 4bit）：

```
bit 0-3:   Fire
bit 4-7:   Lives
bit 8-11:  Speed
bit 12-15: Cooldown
```

---

## ③ View Agent

**目录**：`src/view/` + `include/view/`

### W1 — GameScene 背景缓存系统（已有，补登规格） ✅

**文件**：`GameScene.hpp/cpp`

使用 `QPixmap m_bgCache` 缓存背景绘制，避免每帧重绘渐变+星空贴图：

```cpp
// GameScene.hpp — 背景缓存
QPixmap m_bgCache;
float   m_bgCacheFar  = -999.0f;
float   m_bgCacheNear = -999.0f;
```

**缓存失效条件**：`m_scrollFar` 或 `m_scrollNear` 变化超过 4 像素时重建缓存。缓存内容包含：深空渐变背景 + 星域贴图（远层+近层双层滚动）。

### W2 — 闪烁星星系统（已有，补登规格） ✅

**文件**：`GameScene.cpp` — `kStars[]`

30 颗静态星星，位于屏幕各位置，带参数化闪烁效果：

```cpp
struct StarData {
    float x, y, size;
    int   colorSeed;        // 5 种颜色之一
    float twinkleSpeed;     // 闪烁速度 (0.3~0.8)
};
```

每帧在 `drawForeground()` 中绘制，闪烁公式：`twinkle = 0.6 + 0.4 * sin(time * speed * 3.0 + x * 10.0)`。

### W3 — 技能光环视觉效果（已有，补登规格） ✅

**文件**：`GameScene.cpp` — `drawForeground()` 技能光环部分

5 种技能在玩家周围绘制不同的视觉光环：

| 技能     | 视觉风格                 | 代码行           |
| -------- | ------------------------ | ---------------- |
| 雷暴领域 | 金色旋转光线 + 虚线圆    | `st == 0` 分支 |
| 火焰风暴 | 橙色脉冲光晕 + 旋转火星  | `st == 1` 分支 |
| 极寒护盾 | 蓝色圆环 + 6 条冰晶射线  | `st == 2` 分支 |
| 铁壁阵   | 双层金色圆环（内实外虚） | `st == 4` 分支 |
| 时空闪避 | 淡蓝色小光环             | else 分支        |

### W4 — 护盾指示器 ✅

**文件**：`GameScene.cpp` — `drawForeground()` 护盾部分

当玩家拥有道具护盾且技能未激活时，在玩家周围绘制绿色半透明光环：

```cpp
if (m_pHasShield && *m_pHasShield && (!m_pSkillActive || !*m_pSkillActive))
    // 绿色半透明圆形光环
```

### W5 — Boss 多图片映射（已有，补登规格） ✅

**文件**：`GameScene.cpp` — 精灵绘制中 BOSS 图片选择

根据 `actor.maxHp` 自动选择不同的 BOSS 图片：

```cpp
const QPixmap* bossImg = m_pBossImg;
if (actor.maxHp <= 250 && m_pBossImg2) bossImg = m_pBossImg2;   // 中型BOSS
else if (actor.maxHp <= 400 && m_pBossImg3) bossImg = m_pBossImg3;  // 重型BOSS
else if (m_pBossImg4) bossImg = m_pBossImg4;  // 装甲BOSS
```

迭代2仅定义了单张 BOSS 图片，实际代码通过 `setBossPixmap/2/3/4()` 支持 4 档 BOSS 图片。

### W6 — 半透明 HUD 面板系统（已有，补登规格） ✅

**文件**：`GameScene.cpp` — `drawHudPanel()`

全局 HUD 使用带透明度的半透明圆角面板，统一视觉风格：

```cpp
void drawHudPanel(QPainter* painter, float x, float y, float w, float h, const QColor& color);
```

面板覆盖：分数(左上)、最高分、星核数、波次(中上)、生命(右上)、技能状态(右下)。

### W7 — 爆炸粒子系统（已有，补登规格） ✅

**文件**：`GameScene.hpp/cpp` — `Particle` 结构体和 `updateParticles()`

粒子结构：

```cpp
struct Particle {
    float x, y;
    float vx, vy;
    float life;
    QColor color;
};
```

粒子生命周期：`life` 从 1.0 递减到 0，绘制时透明度按 `life` 缩放，大小按 `2.0 + life * 3.0` 缩放。

**⚠️ 当前粒子仅在 `GameView::tick()` 中调用 `m_scene->updateParticles(0.016f)` 更新位置和生命，但尚未实现敌机死亡时生成粒子的逻辑（迭代7 P1 待补全）**。

### W8 — 技能冷却环形进度条（已有，补登规格） ✅

**文件**：`GameScene.cpp` — `drawForeground()` 技能冷却部分

```cpp
// 使用 drawArc 绘制环形冷却进度
int span = static_cast<int>(360.0f * (1.0f - pct) * 16);
painter->drawArc(QRectF(cx - rr, cy - rr, rr * 2, rr * 2), 90 * 16, -span);
```

- 冷却中：灰色圆环 + 蓝色填充弧 + 百分比文字
- 准备就绪：绿色脉冲闪烁文字 "[SPACE] 释放技能"
- 激活中：金色文字 "✦ 技能激活中 ✦"

### W9 — 雷击特效绘制（已有，补登规格） ✅

**文件**：`GameScene.cpp` — `drawForeground()` 雷击部分

```cpp
if (m_pThunderActive && *m_pThunderActive) {
    // 1. 全屏闪白（透明度 60/255）
    painter->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(255, 255, 255, 60));
    // 2. 4 条锯齿闪电从上到下
    for (int b = 0; b < 4; ++b) {
        // 锯齿闪电：每段横向随机偏移 + 正弦扰动
    }
}
```

- 触发时机：`GameMapVM::handleThunderStrike()` 设置 `m_thunderActive = true`
- 持续时间：0.3~0.4 秒（`m_thunderTimer` 控制）
- `GameView::tick()` 中每帧触发 `GameScene::updateParticles()` 同时更新计时

### W10 — 胜利结算界面 LevelCompleteScreen（已有，补登规格） ✅

**文件**：`LevelCompleteScreen.hpp/cpp`

**页面索引**：`GameView` 页面栈 index 8

```cpp
// LevelCompleteScreen.hpp
class LevelCompleteScreen : public QWidget {
    void setScore(int score);
    void setHighScore(int score);
    void setLevel(int level);
    void setStats(int enemiesKilled, int bossesKilled, float timePlayed);
    void setHighScoreText(const QString& text);
signals:
    void nextLevelClicked();      // 下一关
    void backToMenuClicked();     // 返回菜单
};
```

**⚠️ 当前状态**：`GameView` 中创建了 `LevelCompleteScreen` 并连接了信号，但 `GameMapVM::tickImpl()` 在闯关完成时设置 `m_state = GameState::GameOver` 而非 `GameState::LevelComplete`（复用 GameOver 界面显示通关信息），`updatePage()` 中 `case GameState::LevelComplete` 仅做安全兜底跳转到 index 5。这导致 `LevelCompleteScreen` 目前未实际使用。

### W11 — 加载过渡画面 SplashScreen（已有，补登规格） ✅

**文件**：`SplashScreen.hpp/cpp`

**页面索引**：`GameView` 页面栈 index 9

```cpp
class SplashScreen : public QWidget {
    void setMessage(const QString& msg);    // 设置加载文字
    void setProgress(int percent);           // 设置进度 0~100
};
```

目前已在 `GameView` 构造函数中创建并添加到页面栈，但尚未接入页面切换流程。

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### R1 — 分关卡最高分系统（已有，补登规格） ✅

**文件**：`SaveManager.hpp/cpp`

```cpp
int  loadCampaignHighScore(int level);       // level 1-7
void saveCampaignHighScore(int level, int score);
int  loadEndlessHighScore();
void saveEndlessHighScore(int score);
```

JSON 格式：每个关卡独立存储最高分，无尽模式单独存储。

### R2 — 5 战机独立升级数据打包存储（已有，补登规格） ✅

**文件**：`SaveManager.hpp/cpp`

```cpp
struct UpgradeData {
    int starCores = 0;
    int levelsPacked[5] = {};   // 5 架战机各 16bit
};
```

存档加载流程：`AppAgent::init()` → `SaveManager::loadUpgradeData()` → `GameMapVM::initUpgradeData()`。

存档保存流程：`GameMapVM::tickImpl()` 游戏结束时 → `emit saveUpgradeRequested()` → `AppAgent` 中转 → `SaveManager::saveUpgradeData()`。

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### A1 — 完整持久化链条（已有，补登规格） ✅

**文件**：`AppAgent.cpp`

三种持久化请求的完整连接：

| 信号                       | 触发时机            | 保存内容                |
| -------------------------- | ------------------- | ----------------------- |
| `saveHighScoreRequested` | 关卡完成 / 游戏结束 | 对应关卡/无尽最高分     |
| `saveCampaignRequested`  | 关卡完成            | 已解锁的最高关卡数      |
| `saveUpgradeRequested`   | 游戏结束 / 升级操作 | 星核数 + 5 战机升级数据 |
| `resetAllRequested`      | 重置数据            | 清空所有存档            |

### A2 — 战机选择数据注入（已有，补登规格） ✅

**文件**：`AppAgent.cpp` — `init()` 中

```cpp
// 从 AircraftStats 模板读取数据，注入 AircraftSelectScreen
static const char* kSkillNames[5] = {"雷暴领域","火焰风暴","极寒护盾","时空闪避","铁壁守护"};
static const char* kDescs[5] = {
    "均衡旗舰 · 无短板","极致火力 · 高输出","最强生存 · 稳如山",
    "极速游击 · 风筝王","钢铁壁垒 · 稳扎稳打"};
for (int i = 0; i < 5; ++i) {
    const auto& t = AircraftStats::getTemplate(static_cast<AircraftType>(i));
    m_gameView->setAircraftCardData(i, t.name, t.starFirePower,
        t.baseLives, static_cast<int>(t.skillCooldown),
        kSkillNames[i], kDescs[i]);
}
```

此模式避免 View 硬编码战机数据，同时让 View 不依赖 `AircraftStats` 头文件。

---

## ⑥ 已修复遗留问题（迭代 8 已修复）

| # | 问题 | 状态 | 修复说明 |
| - | ---- | ---- | -------- |
| 1 | `LevelCompleteScreen` 已创建但未实际使用 — `GameMapVM` 闯关完成时用 `GameOver` 状态代替 | ✅ **已修复** | `GameMapVM::tickImpl()` 闯关完成时改为 `m_state = GameState::LevelComplete`；`GameView::updatePage()` 中 `LevelComplete` 切换到 index 8 并填充统计数据 |
| 2 | `SplashScreen` 已创建但未接入页面切换流程 | ✅ **已修复** | 战机选择确认后和下一关点击时先显示 SplashScreen（index 9），600ms 后自动进入 Playing 状态 |
| 3 | 粒子系统仅有更新逻辑 (`updateParticles`) 但未实现生成逻辑 — 敌机死亡时未调用生成粒子 | ✅ **已修复** | 新增 `ActorType::Explosion`；`GameMapVM` 在碰撞检测/BOSS死亡/雷击/冲刺时收集爆炸位置同步到 AirMap；`GameScene` 读取后生成 8~12 个扩散光点粒子 |
| 4 | `Constants.hpp` 已从 Common 移除但迭代文件仍引用 | ⏸️ 用户认为无需处理 | `viewmodel/GameConstants.hpp` 和 `view/ViewConstants.hpp` 已是实际代码状态 |
| 5 | `GameMapVM::tickImpl()` 中雷击特效计时出现两次 | ✅ **已修复** | 删除 tickImpl 中第 2 步的重复计时代码，保留第 15 步的唯一版本 |

---

## 开发顺序（补全遗留问题）

```
① View (P1 粒子生成 → W10 LevelComplete 接入 → W11 SplashScreen 接入)  ← 主力
     ↓
② ViewModel (P1 关卡完成状态修复 → P5 重复代码清理)
     ↓
③ App (页面切换适配)
     ↓
④ 所有 Agent (P4 文档与代码对齐)
```
