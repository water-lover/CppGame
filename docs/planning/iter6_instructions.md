# 迭代 6 — 升级系统 + 无尽模式重做：Agent 指令

> **整体目标**：实现升级系统（通用）和重做无尽模式难度曲线
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 迭代 6 节和 `docs/planning/RULES.md`
>
> **核心设计**：
> - 升级系统**不附属于无尽模式**，闯关和无尽均可获取材料并升级
> - 无尽模式是闯关模式的循环，每轮递增难度
>
> **状态标记**：❌ = 待完成

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

### 需要修改的文件

| 文件 | 变更 | 状态 |
|---|---|---|
| `include/common/Constants.hpp` | 新增升级相关常量 | ❌ |
| `include/common/PropertyIds.hpp` | 新增 `PROP_ID_STAR_CORES`、`PROP_ID_UPGRADE_LEVELS` | ❌ |

**Constants.hpp 新增：**
```cpp
// ── 升级系统常量 ───────────────────────────────────────────
constexpr int   MAX_UPGRADE_LEVEL    = 10;    // 每项属性最大等级
constexpr int   STAR_CORE_PER_KILL   = 1;     // 每击杀一个敌人获得的星核数
constexpr int   STAR_CORE_PER_BOSS   = 10;    // 每击败一个 BOSS 获得的星核数
constexpr float UPGRADE_FIRE_DELTA   = 0.5f;  // 每级火力提升（武器等级系数）
constexpr int   UPGRADE_LIVES_DELTA  = 1;     // 每级生命提升
constexpr float UPGRADE_SPEED_DELTA  = 0.05f; // 每级速度提升
constexpr float UPGRADE_COOLDOWN_DELTA = 0.05f; // 每级冷却缩减比例
```

**PropertyIds.hpp 新增：**
```cpp
PROP_ID_STAR_CORES,         // 星核数量变化
PROP_ID_UPGRADE_LEVELS,     // 升级等级变化
```

---

## ② ViewModel Agent（主力）

**目录**：`src/viewmodel/` + `include/viewmodel/`

### 需要创建的文件 ❌

| 文件 | 内容 |
|---|---|
| `include/viewmodel/UpgradeManager.hpp` | 升级管理器 — 管理星核和升级数据 |
| `src/viewmodel/UpgradeManager.cpp` | 实现升级逻辑（消耗星核、计算属性加成） |

### 需要修改的文件 ❌

| 文件 | 变更 |
|---|---|
| `GameMapVM.hpp/cpp` | 集成 UpgradeManager、星核掉落收集、升级命令 |
| `WaveManager.cpp` | 重做无尽模式难度曲线 |
| `PowerUpManager.hpp/cpp` | 新增星核碎片掉落类型 |

---

### UpgradeManager 设计

**头文件接口：**

```cpp
// UpgradeManager.hpp
#ifndef UPGRADEMANAGER_HPP
#define UPGRADEMANAGER_HPP

/// 可升级的属性
enum class UpgradeType {
    FirePower,    // 火力
    Lives,       // 生命
    Speed,       // 速度
    Cooldown     // 冷却
};

/// 升级系统管理器
///
/// 管理星核碎片和 4 项属性的升级等级。
/// 闯关模式和无尽模式共用升级数据。
class UpgradeManager {
public:
    UpgradeManager() = default;

    // ── 星核管理 ──────────────────────────────────────────────
    int  getStarCores()     const { return m_starCores; }
    void addStarCores(int amount) { m_starCores += amount; }
    bool spendStarCores(int amount);  // 返回是否成功

    // ── 升级管理 ──────────────────────────────────────────────
    int  getUpgradeLevel(UpgradeType type) const;
    void setUpgradeLevel(UpgradeType type, int level);
    bool upgrade(UpgradeType type);  // 尝试升级，消耗星核，返回成功

    /// 获取某属性的总加成值
    float getFirePowerBonus()  const;  // 返回累加值（如 1.5 = +50% 火力等级）
    int   getLivesBonus()      const;  // 返回额外生命数
    float getSpeedBonus()      const;  // 返回速度倍率加成
    float getCooldownBonus()   const;  // 返回冷却缩减比例 [0,1]

    // ── 序列化（供 SaveManager 读写） ─────────────────────────
    int   getLevelsData()     const;  // 将 4 项等级打包为 int（每项 4bit）
    void  setLevelsData(int data);    // 解包

    static constexpr int MAX_LEVEL  = 10;

private:
    int m_starCores = 0;
    int m_fireLevel    = 0;
    int m_livesLevel   = 0;
    int m_speedLevel   = 0;
    int m_cooldownLevel = 0;
};

#endif // UPGRADEMANAGER_HPP
```

**升级消耗公式：**
```cpp
// 每级消耗的星核数：10 × (当前等级 + 1)
// 等级0→1 需 10 个，等级1→2 需 20 个，...等级9→10 需 100 个
bool UpgradeManager::upgrade(UpgradeType type) {
    int* level = nullptr;
    switch (type) {
    case UpgradeType::FirePower:  level = &m_fireLevel; break;
    case UpgradeType::Lives:      level = &m_livesLevel; break;
    case UpgradeType::Speed:      level = &m_speedLevel; break;
    case UpgradeType::Cooldown:   level = &m_cooldownLevel; break;
    }
    if (!level || *level >= MAX_LEVEL) return false;
    int cost = 10 * (*level + 1);
    if (m_starCores < cost) return false;
    m_starCores -= cost;
    (*level)++;
    return true;
}
```

---

### PowerUpManager 扩展

在 `PowerUpManager::PowerUpType` 中新增 `StarCore` 类型，掉落概率整合：

```cpp
// PowerUpManager.hpp
enum class PowerUpType {
    StarCore,   // 星核碎片 ← 新增（~40% 概率）
    Hp,         // 回血（~25%）
    Fire,       // 火力加强（~20%）
    Shield      // 护盾（~15%）
};
```

**掉落逻辑调整：**
- 普通敌机死亡：`randomDrop()` 按上述概率掉落
- BOSS 死亡：必定掉落 `StarCore` + 额外随机道具

---

### WaveManager — 无尽模式重做

**当前无尽逻辑**：
- 每轮 `endlessLoop_` 递增，`spawnInterval *= (1 - loop * 0.05)`，`enemySpeed *= (1 + loop * 0.1)`
- 循环使用第 7 关配置

**新无尽逻辑（按用户要求）：**

```
Wave 1 (loop=1): levels 1-7, 难度 = 闯关的 50% (enemySpeed × 0.5, hpBonus 减半)
Wave 2 (loop=2): levels 1-7, 难度 = 闯关的 100% (与闯关一致)
Wave 3 (loop=3): levels 1-7, 难度 = 闯关的 150% (100% + 0.5×)
Wave 4 (loop=4): levels 1-7, 难度 = 闯关的 200% (100% + 1.0×)
...
Wave n (loop=n): 难度 = 100% + (n-2) × 50%
```

**WaveManager::update 修改：**

```cpp
void WaveManager::update(/*...*/) {
    // 无尽模式：计算当前波次的难度倍率
    float diffMult = 1.0f;
    if (endlessMode_) {
        // loop=1 → 0.5, loop=2 → 1.0, loop=3 → 1.5, loop=4 → 2.0 ...
        diffMult = 0.5f + (endlessLoop_ - 1) * 0.5f;
        if (diffMult < 0.5f) diffMult = 0.5f;
    }

    // 应用难度倍率
    float actualInterval = cfg.spawnInterval / (1.0f + (diffMult - 1.0f) * 0.2f);
    float actualSpeed   = cfg.enemySpeed * diffMult;
    int   actualHpBonus = static_cast<int>(cfg.enemyHpBonus * diffMult);
    float actualBossHp  = (bossHp_ * diffMult);  // BOSS 血量也按倍率增加
    // ...
}
```

**重置无尽关卡的逻辑**：打完第 7 关后重置到第 1 关，`endlessLoop_++`，显示提示 "进入 Wave N"。

---

### GameMapVM — 集成升级

**新增命令：**

```cpp
// GameMapVM.hpp — 新增
std::function<void()>         getUpgradeStatCommand(int type);  // type = UpgradeType
```

**新增属性：**

```cpp
// GameMapVM.hpp — 新增
const int* getStarCoresPtr()    const noexcept;
const int* getUpgradeLevelsPtr() const noexcept;  // 打包的 4 项等级数据
```

**tickImpl 中星核掉落：**

在敌机死亡时（checkCollisions 中），调用 `m_upgradeMgr.addStarCores(count)` 并发射 `PROP_ID_STAR_CORES`：
```cpp
// checkCollisions 中，敌机死亡时
if (m_enemies[ei]->isDead()) {
    m_scoreMgr.addScore(m_enemies[ei]->getScore());
    m_upgradeMgr.addStarCores(STAR_CORE_PER_KILL);  // 击杀获得星核
    m_powerUpMgr.onEnemyDestroyed(m_enemies[ei]->getPos(), m_rng);
    fireChange(PROP_ID_STAR_CORES);
}

// BOSS 死亡时（在 cleanup 前的 BOSS 检测中）
if (boss && boss->isDead()) {
    m_upgradeMgr.addStarCores(STAR_CORE_PER_BOSS);
    fireChange(PROP_ID_STAR_CORES);
}
```

**升级命令实现：**

```cpp
void GameMapVM::upgradeStatImpl(int type) {
    auto ut = static_cast<UpgradeType>(type);
    if (m_upgradeMgr.upgrade(ut)) {
        // 应用升级效果到玩家属性
        fireChange(PROP_ID_UPGRADE_LEVELS);
        fireChange(PROP_ID_STAR_CORES);
        log("GameMapVM", "Upgrade " + std::to_string(type) + " success");
    }
}
```

**升级属性应用到玩家：**
- `startGameImpl()` 中，在重置玩家后调用升级加成：
```cpp
// 应用升级加成
m_player.setWeaponLevel(1 + static_cast<int>(m_upgradeMgr.getFirePowerBonus()));
// 生命加成由 AircraftStats.baseLives + UpgradeManager.getLivesBonus()
// 速度加成由 AircraftStats.speedMultiplier + UpgradeManager.getSpeedBonus()
```

由于 Player 当前的生命/速度是基于 AircraftStats 模板的，升级加成需要叠加。最简方式：升级加成作为额外属性附加到 Player。

**修改 Player 的 getLives/getSpeedValue：**

```cpp
// Player.hpp — 新增
void  setUpgradeBonuses(float fireBonus, int livesBonus, float speedBonus, float cdBonus);

// Player.cpp 中重置时应用
void Player::reset() {
    // ... 原有初始化 ...
    m_weaponLevel = 1 + static_cast<int>(m_fireBonus);
    m_lives = getMaxLives() + m_livesBonus;
}
```

---

## ③ View Agent

**目录：** `src/view/` + `include/view/`

### 需要创建的文件 ❌

| 文件 | 内容 |
|---|---|
| `include/view/UpgradeScreen.hpp` | 升级界面 — 显示 4 项可升级属性+星核数量 |
| `src/view/UpgradeScreen.cpp` | 升级界面实现 |

### 需要修改的文件 ❌

| 文件 | 变更 |
|---|---|
| `include/view/GameView.hpp` | 新增 UpgradeScreen 页面索引 |
| `src/view/GameView.cpp` | 构造函数创建 UpgradeScreen；新增升级命令 setter |
| `include/view/HudOverlay.hpp` | 新增星核数量显示 |
| `src/view/HudOverlay.cpp` | 绘制星核数量 |

---

### UpgradeScreen 设计

**访问入口：** 从 ModeSelectScreen 或 LevelSelectScreen 进入，也可在游戏内暂停时进入。

**页面布局：**
```
┌──────────────────────────────────────────┐
│              升  级  系  统               │
│                                           │
│  星核碎片: ★★★ 250 个                    │
│                                           │
│  ┌──────────────────────────────────┐     │
│  │  火力  Lv.3  →  Lv.4  [升级]     │     │
│  │  +1.5 武器等级  需 40 星核        │     │
│  ├──────────────────────────────────┤     │
│  │  生命  Lv.2  →  Lv.3  [升级]     │     │
│  │  +2 生命上限    需 30 星核        │     │
│  ├──────────────────────────────────┤     │
│  │  速度  Lv.1  →  Lv.2  [升级]     │     │
│  │  +0.05 速度     需 20 星核        │     │
│  ├──────────────────────────────────┤     │
│  │  冷却  Lv.0  →  Lv.1  [升级]     │     │
│  │  -5% 冷却时间   需 10 星核        │     │
│  └──────────────────────────────────┘     │
│                                           │
│  [返  回]                                  │
└──────────────────────────────────────────┘
```

**核心机制：**
- 显示当前星核数量
- 4 项属性各显示当前等级、下一级效果、升级所需星核
- 点击 [升级] 按钮 → 调用 `upgradeStatCommand(type)`
- 星核不够时按钮灰色不可点
- 满级时显示 "MAX"

**头文件接口：**

```cpp
// UpgradeScreen.hpp
class UpgradeScreen : public QWidget {
    Q_OBJECT

public:
    explicit UpgradeScreen(QWidget* parent = nullptr);

    /// 注入升级命令
    void setUpgradeStatCommand(std::function<void(int)>&& cmd);

    /// 更新显示数据
    void setStarCores(int count);
    void setUpgradeLevel(int type, int level);  // type=UpgradeType, level=0~10

signals:
    void backClicked();

private:
    struct UpgradeSlot {
        UpgradeType type;
        const char* name;
        const char* desc;
        QPushButton* btn;
        QLabel* infoLabel;
    };

    void setupUI();
    void refreshSlot(UpgradeSlot& slot);

    std::function<void(int)> m_upgradeStatCommand;
    int m_starCores = 0;
    int m_levels[4] = {};  // Fire, Lives, Speed, Cooldown
    QVector<UpgradeSlot> m_slots;
    QPushButton* m_backBtn = nullptr;
};
```

**GameView 页面索引更新：**

当前页面栈：
```
0: StartScreen
1: ModeSelectScreen
2: LevelSelectScreen
3: AircraftSelectScreen
4: GamePage
5: GameOverScreen
6: PauseOverlay
```

新增页面 7: UpgradeScreen（放在 Pause 之后或 ModeSelect 之前）：
```
7: UpgradeScreen ← 新增
```

访问方式：从 ModeSelectScreen/LevelSelectScreen 加一个"升级"按钮。或者在 ModeSelectScreen 中加一个入口。

**HUD 星核显示：**

在 GameScene 的 `drawForeground` 中，左上角显示星核数量：
```cpp
// 星核（分数下方）
if (m_pStarCores) {
    painter->setPen(QColor(100, 200, 255, 200));
    QFont sf(QStringLiteral("Microsoft YaHei"), 13);
    painter->setFont(sf);
    painter->drawText(QRectF(14, 70, 200, 26), Qt::AlignLeft | Qt::AlignVCenter,
                      QString("★ %1").arg(*m_pStarCores));
}
```

GameScene 新增：
```cpp
void setHudStarCores(const int* p) noexcept { m_pStarCores = p; }
const int* m_pStarCores = nullptr;
```

---

## ④ Resource Agent

**目录：** `src/resource/` + `include/resource/`

### 需要修改的文件 ❌

| 文件 | 变更 |
|---|---|
| `include/resource/SaveManager.hpp` | 新增升级数据读写 |
| `src/resource/SaveManager.cpp` | 实现升级数据的序列化 |

```cpp
// SaveManager.hpp 新增
struct UpgradeData {
    int starCores = 0;
    int levelsData = 0;   // 4 项等级打包
};

UpgradeData loadUpgradeData();
void saveUpgradeData(const UpgradeData& data);
```

JSON 格式：
```json
{
    "highScore": 1000,
    "campaignLevel": 3,
    "starCores": 250,
    "upgradeLevels": 173     // 打包：每项 4bit
}
```

**打包/解包：**
```cpp
// levelsData 编码：每项用 4bit（0~15），共 16bit
// bit 0-3: FirePower, bit 4-7: Lives, bit 8-11: Speed, bit 12-15: Cooldown
int packLevels(int fire, int lives, int speed, int cd) {
    return (fire & 0xF) | ((lives & 0xF) << 4) |
           ((speed & 0xF) << 8) | ((cd & 0xF) << 12);
}
```

---

## ⑤ App Agent

**目录：** `src/app/` + `include/app/`

### 需要修改的文件 ❌

| 文件 | 变更 |
|---|---|
| `src/app/AppAgent.cpp` | 注入升级命令、桥接星核/升级指针、持久化升级数据 |

**新增绑定：**
```cpp
// 升级命令
m_gameView->setUpgradeStatCommand(m_mapVM->getUpgradeStatCommand());

// 属性指针
m_gameView->setStarCoresPtr(m_mapVM->getStarCoresPtr());

// 持久化：GameOver 时保存升级数据
QObject::connect(m_mapVM, &GameMapVM::saveUpgradeRequested,
    [](int starCores, int levelsData) {
        SaveManager().saveUpgradeData({starCores, levelsData});
    });

// 初始化时加载升级数据
auto upgradeData = SaveManager().loadUpgradeData();
// 注入到 UpgradeManager...
```

---

## ⑥ Test Agent

**目录：** `tests/`

### 需要创建的文件 ❌

| 文件 | 内容 |
|---|---|
| `tests/test_upgrade_manager.cpp` | UpgradeManager 升级消耗、星核加减、等级上限测试 |

### 需要扩展的文件 ❌

| 文件 | 新增内容 |
|---|---|
| `tests/test_wave_manager.cpp` | 新增无尽模式难度曲线测试（0.5x / 1.0x / 1.5x ...） |
| `tests/test_game_map_vm.cpp` | 新增星核掉落、升级命令集成测试 |

---

## 开发顺序

```
② ViewModel (UpgradeManager → PowerUpManager 星核 → WaveManager 无尽重做 → GameMapVM 集成)  ← 主力
     ↓
① Common (Constants + PropertyIds)
     ↓
③ View (UpgradeScreen 新建 + HUD 星核 + GameView 页面)
     ↓
④ Resource (SaveManager 升级数据持久化)
     ↓
⑤ App (组装)
     ↓
⑥ Test (全覆盖)
```

---

## 流程总结

```
闯关模式：
  ModeSelect → LevelSelect → AircraftSelect → Game
     ↑                                    ↑
     └──── 升级入口 ← UpgradeScreen ──────┘

无尽模式：
  ModeSelect → AircraftSelect → Game(Level 1~7 循环)
     ↑                              ↑
     └── 升级入口 ← UpgradeScreen ──┘

无尽难度曲线：
  Wave 1: 闯关难度的 50%  (easy)
  Wave 2: 闯关难度的 100% (normal)
  Wave 3: 闯关难度的 150% (hard)
  Wave N: 100% + (N-2)×50%

星核获取：
  击杀小型敌机:  +1 星核
  击杀中型敌机:  +2 星核
  击杀大型/精英: +3 星核
  击杀 BOSS:    +10 星核

升级消耗：
  Lv.N → Lv.N+1: 10 × (N+1) 星核
  单项满级: 10 级
```
