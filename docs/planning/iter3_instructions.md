# 迭代 3 — BOSS 战 + 7 关闯关：Agent 指令（补充版）

> **整体目标**：WaveManager 波次系统 + 敌机/BOSS AI + 道具系统 + 闯关模式
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 和 `docs/planning/RULES.md`
>
> **状态标记**：✅ = 已完成（无需再动） ❌ = 待完成（本次需要做的）

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

### 需要修改的文件

| 文件 | 变更 | 状态 |
|---|---|---|
| `include/common/PropertyIds.hpp` | 新增 `PROP_ID_BOSS_HEALTH`、`PROP_ID_WAVE` | ❌ |
| `include/common/Types.hpp` | 新增 `GameMode` 枚举（`Campaign, Endless`）；`ActorType` 增加 `EnemyMedium, EnemyLarge, Boss` | ❌ |

---

## ② ViewModel Agent（主力）— 工作量最大

**目录**：`src/viewmodel/` + `include/viewmodel/`

### 已完成 ✅

| 文件 | 已实现内容 |
|---|---|
| `Player.hpp/cpp` | 新增 `weaponLevel`（1~5）、`setShielded()`、`m_hasShield` |
| `Bullet.hpp` | 已有 `enum Owner { Player, Enemy }`，敌方子弹可用 |
| `AircraftStats.hpp/cpp` | 5 架战机属性模板（迭代 4 内容，提前完成） |
| `SkillSystem.hpp/cpp` | 5 种主动技能（迭代 4 内容，提前完成） |

### 待完成 ❌

| 文件 | 需要做的事 |
|---|---|
| **`WaveManager.hpp` + `WaveManager.cpp`** | **新建** — 波次管理器（7 关关卡表、每关 3~5 波 + BOSS 波、无尽模式难度递增） |
| **`PowerUpManager.hpp` + `PowerUpManager.cpp`** | **新建** — 道具管理器（15% 掉落概率、回血/火力加强/护盾效果） |
| **`Enemy.hpp` + `Enemy.cpp`** | **重写** — 改为基类 + 虚函数；新增 `EnemySmall`(直线)、`EnemyMedium`(摆动+单发)、`EnemyLarge`(V形散射)、`EnemyElite`(追踪+扇形弹幕) |
| **`Boss.hpp` + `Boss.cpp`** | **新建** — BOSS 类：3 阶段转换（血量 100%/50%/25%），每阶段不同攻击模式（单发/双发/散弹/瞄准弹/全屏弹幕/召唤） |
| **`Bullet.hpp/cpp`** | 扩展：区分玩家子弹（蓝色）和敌方子弹（红色），可用 `owner` 字段控制绘制 |
| **`SpiritVM.hpp/cpp`** | 新增 setter/getter：`setEnemyMediumPixmap`、`setEnemyLargePixmap`、`setBossPixmap`、`setPowerUpHpPixmap`、`setPowerUpFirePixmap`、`setPowerUpShieldPixmap` |
| **`GameMapVM.hpp/cpp`** | 集成 `WaveManager`、`PowerUpManager`；新增 `m_bossHp`/`m_bossMaxHp`；`tickImpl()` 中调用波次和道具逻辑；新增 `getWave()`/`getBossHp()`/`getBossMaxHp()`/`getGameMode()`；`syncMap()` 中同步 BOSS 和道具到 AirMap |

### 代码参考

<details>
<summary>WaveManager 接口（点击展开）</summary>

```cpp
struct WaveConfig {
    int levelId;
    int waveCount;          // 3~5
    float spawnInterval;
    float enemySpeed;       // 倍率
    int enemyHpBonus;
    bool hasBoss;
    int bossId;
};

class WaveManager {
public:
    void reset(int levelId);
    void update(float dt, std::vector<std::unique_ptr<Enemy>>& enemies);
    bool isWaveComplete() const;
    bool isLevelComplete() const;
    int  getCurrentWave() const;
    int  getCurrentLevel() const;
    WaveConfig getConfigForLevel(int levelId);

private:
    float m_timer = 0.0f;
    int m_currentWave = 0, m_currentLevel = 1;
    int m_waveEnemiesSpawned = 0, m_waveEnemiesTotal = 0;
    bool m_bossSpawned = false, m_bossDefeated = false;
};

// 7 关配置表
const WaveConfig LEVEL_TABLE[7] = {
    {1, 3, 1.5, 1.0, 0, true, 1},
    {2, 3, 1.4, 1.1, 0, true, 2},
    {3, 4, 1.3, 1.2, 1, true, 2},
    {4, 4, 1.2, 1.3, 1, true, 3},
    {5, 5, 1.1, 1.4, 2, true, 3},
    {6, 5, 1.0, 1.5, 3, true, 4},
    {7, 5, 0.9, 1.8, 5, true, 4},
};
```
</details>

<details>
<summary>PowerUpManager 接口（点击展开）</summary>

```cpp
enum class PowerUpType { Hp, Fire, Shield };

class PowerUpManager {
public:
    void update(float dt, const std::vector<std::unique_ptr<Enemy>>& enemies,
                std::vector<Bullet>& bullets);
    void onEnemyDestroyed(const Enemy& enemy, std::vector<Actor>& actors);
    static constexpr float DROP_CHANCE = 0.15f;
    static constexpr float FIRE_BOOST_DURATION = 15.0f;
private:
    PowerUpType randomDrop();
};
```
</details>

<details>
<summary>Enemy 基类 + 子类架构（点击展开）</summary>

```cpp
class Enemy {
public:
    Enemy() = default;
    Enemy(float x, float y, float speed);
    virtual ~Enemy() = default;
    virtual void update(float dt);
    virtual bool canAttack(float dt);
    virtual void attack(std::vector<Bullet>& bullets);

    Vec2  getPos() const;
    float getSize() const;
    int   getHp() const;
    void  takeDamage();
    bool  isDead() const;
    bool  isOffScreen() const;
    virtual int getScore() const;
    // ...

protected:
    Vec2  pos_;
    float speed_ = 0.25f;
    float size_  = 0.05f;
    int   hp_    = 1;
    float attackTimer_ = 0.0f;
};

// 小型机 — 直线下飞，不攻击
class EnemySmall : public Enemy { /* 继承默认 */ };

// 中型机 — 左右正弦摆动，每 2s 单发
class EnemyMedium : public Enemy { /* override update/attack */ };

// 大型机 — 缓慢移动，每 1.5s V 形双发
class EnemyLarge : public Enemy { /* override update/attack */ };

// 精英机 — 追踪玩家 X，每 1s 3 发散弹
class EnemyElite : public Enemy { /* override update/attack */ };
```
</details>

<details>
<summary>BOSS 设计（点击展开）</summary>

```cpp
enum class BossPhase { Phase1, Phase2, Phase3 };
enum class AttackType { Single, Double, Spread, Aimed, Barrage, Summon };

struct BossAttackPattern {
    AttackType type;
    float interval;
    int bulletCount;
    float spreadAngle;
    float bulletSpeed;
};

class Boss : public Enemy {
    BossPhase m_phase = BossPhase::Phase1;
    std::vector<BossAttackPattern> m_phasePatterns[3];
    float m_attackTimer = 0.0f;
    int m_maxHp = 50;
    
    void update(float dt) override;
    void updatePhase();
    void executeAttack(const BossAttackPattern& pattern, std::vector<Bullet>& bullets);
};

// 全屏弹幕函数
void spawnCircularBarrage(float cx, float cy, int count, float speed, std::vector<Bullet>& bullets);
void spawnSpiralBarrage(float cx, float cy, int count, float speed, float baseAngle, std::vector<Bullet>& bullets);
```
</details>

---

## ③ View Agent

**目录**：`src/view/` + `include/view/`

### 已完成 ✅

| 文件 | 已实现内容 |
|---|---|
| `BossHealthBar.hpp/cpp` | BOSS 血条控件：红色血条 + HP 数值显示 |

### 待完成 ❌

| 文件 | 需要做的事 |
|---|---|
| `src/view/GameScene.cpp` | 扩展 `drawForeground`：绘制中型机（红色图）、大型机（黑色图）、BOSS（2x 大小）、道具（不同颜色图标）；根据 `ActorType` 选择对应图片和尺寸 |
| `src/view/EnemyItem.cpp` | 扩展：支持不同尺寸的敌机绘制（小型 1x、中型 1.2x、大型 1.5x、BOSS 2x） |
| `src/view/BulletItem.cpp` | 扩展：玩家子弹蓝色，敌方子弹红色 |
| `include/view/GameView.hpp/cpp` | 新增 `setBossHpPtr`/`setBossMaxHpPtr` 转发到 `BossHealthBar`；将 `BossHealthBar` 叠加到游戏页面上方 |

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### 待完成 ❌

| 文件 | 需要做的事 |
|---|---|
| `src/resource/SaveManager.cpp` | 扩展：`saveCampaignProgress(int level)` / `loadCampaignProgress()`；存档 JSON 增加 `campaignLevel` 字段 |

QRC 资源（迭代 2 已添加）：确认 `resources/resources.qrc` 已包含 `enemyMedium`、`enemyLarge`、`bossShip`、`bossRocket`、`enemyBullet`、`powerUpHp`、`powerUpFire`、`powerUpShield`。

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### 待完成 ❌

`src/app/AppAgent.cpp`：

```cpp
void AppAgent::init() {
    // ... 迭代 1~2 原有逻辑 ...

    // 桥接变量新增
    m_bridgeWave      = 0;
    m_bridgeBossHp    = 0;
    m_bridgeBossMaxHp = 0;

    // SpiritVM 注入新图片（迭代 3 新增的敌机/BOSS/道具图）
    m_spriteVM->setEnemyMediumPixmap(assets.getImage("enemyMedium"));
    m_spriteVM->setEnemyLargePixmap(assets.getImage("enemyLarge"));
    m_spriteVM->setBossPixmap(assets.getImage("bossShip"));
    m_spriteVM->setEnemyBulletPixmap(assets.getImage("enemyBullet"));
    m_spriteVM->setPowerUpHpPixmap(assets.getImage("powerUpHp"));
    m_spriteVM->setPowerUpFirePixmap(assets.getImage("powerUpFire"));
    m_spriteVM->setPowerUpShieldPixmap(assets.getImage("powerUpShield"));

    // 新增属性绑定
    m_gameView->setWavePtr(&m_bridgeWave);
    m_gameView->setBossHpPtr(&m_bridgeBossHp);
    m_gameView->setBossMaxHpPtr(&m_bridgeBossMaxHp);
}

// onViewModelChanged 扩展
void AppAgent::onViewModelChanged(uint32_t propertyId) {
    // ... 原有逻辑（SCORE / LIVES / GAME_STATE）...
    case PROP_ID_BOSS_HEALTH:
        m_bridgeBossHp = m_mapVM->getBossHp();
        m_bridgeBossMaxHp = m_mapVM->getBossMaxHp();
        break;
    case PROP_ID_WAVE:
        m_bridgeWave = m_mapVM->getWave();
        break;
}
```

---

## ⑥ Test Agent

**目录**：`tests/`

### 待创建 ❌

| 文件 | 内容 |
|---|---|
| `tests/test_wave_manager.cpp` | WaveManager 波次生成、关卡配置表、BOSS 触发、无尽模式难度递增 |
| `tests/test_power_up.cpp` | PowerUpManager 掉落概率分布、拾取效果（回血/火力/护盾） |
| `tests/test_boss.cpp` | BOSS 阶段转换（血量阈值）、攻击模式切换、弹幕生成验证 |

### 待扩展 ❌

| 文件 | 新增内容 |
|---|---|
| `tests/test_game_map_vm.cpp` | 新增模式选择、闯关流程、BOSS 战集成 |
| `tests/test_collision.cpp` | 新增敌方子弹 vs 玩家碰撞、道具拾取碰撞 |

---

## 开发顺序（只标记 ❌ 项）

```
① Common (PropertyIds + Types 新枚举)  — 5 分钟
     ↓
② ViewModel (WaveManager → Enemy/Boss → PowerUpManager → GameMapVM) ← 主力，~2 小时
     ↓
④ Resource (SaveManager 关卡存档)  — 10 分钟，可与 ViewModel 并行
     ↓
③ View (GameScene绘制扩展 → BossHealthBar集成 → BulletItem颜色)  — 30 分钟
     ↓
⑤ App (组装新绑定)  — 10 分钟
     ↓
⑥ Test (全流程覆盖)  — 30 分钟
```
