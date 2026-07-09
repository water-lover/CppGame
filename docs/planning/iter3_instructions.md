# 迭代 3 — BOSS 战 + 7 关闯关：Agent 指令

> **整体目标**：WaveManager 波次系统 + 敌机/BOSS AI + 道具系统 + 闯关模式
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 和 `docs/planning/RULES.md`

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

### 需要修改的文件

| 文件 | 变更 |
|---|---|
| `include/common/PropertyIds.hpp` | 新增 `PROP_ID_BOSS_HEALTH`（BOSS 血量变化）、`PROP_ID_WAVE`（波次变化） |
| `include/common/Types.hpp` | 新增 `GameMode` 枚举（`Campaign, Endless`）；`ActorType` 增加 `EnemyMedium, EnemyLarge, Boss` |

---

## ② ViewModel Agent（主力）

**目录**：`src/viewmodel/` + `include/viewmodel/`

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `include/viewmodel/WaveManager.hpp` + `src/viewmodel/WaveManager.cpp` | **新建** — 波次管理器 |
| `include/viewmodel/PowerUpManager.hpp` + `src/viewmodel/PowerUpManager.cpp` | **新建** — 道具管理器 |

### 需要修改的文件

| 文件 | 变更 |
|---|---|
| `include/viewmodel/Enemy.hpp` + `src/viewmodel/Enemy.cpp` | 扩展为基类；新增中型机、大型机、精英机、BOSS 子类；攻击接口 |
| `include/viewmodel/Bullet.hpp` + `src/viewmodel/Bullet.cpp` | 扩展敌方子弹类型 |
| `include/viewmodel/Player.hpp` + `src/viewmodel/Player.cpp` | 新增 `weaponLevel`（1~5）、`shieldTimer` |
| `include/viewmodel/GameMapVM.hpp` + `src/viewmodel/GameMapVM.cpp` | 集成 WaveManager / PowerUpManager；BOSS 战；关卡过渡 |
| `include/viewmodel/SpiritVM.hpp` + `src/viewmodel/SpiritVM.cpp` | 新增中型机/大型机/BOSS/道具图片的 setter/getter |

---

### WaveManager 设计

```cpp
struct WaveConfig {
    int levelId;            // 关卡 1~7
    int waveCount;          // 小怪波次 3~5
    float spawnInterval;    // 生成间隔
    float enemySpeed;       // 敌机速度倍率
    int enemyHpBonus;       // 敌机额外生命
    bool hasBoss;           // 本关是否有 BOSS
    int bossId;             // BOSS 类型 ID
};

class WaveManager {
public:
    void reset(int levelId);                          // 开始新关卡
    void update(float dt, std::vector<Enemy>& enemies); // 驱动生成逻辑
    bool isWaveComplete() const;                      // 当前波次完成
    bool isLevelComplete() const;                     // 关卡完成（含BOSS）
    int  getCurrentWave() const;
    int  getCurrentLevel() const;

    WaveConfig getConfigForLevel(int levelId);        // 获取关卡配置

private:
    float m_timer = 0.0f;
    int m_currentWave = 0;
    int m_currentLevel = 1;
    int m_waveEnemiesSpawned = 0;
    int m_waveEnemiesTotal = 0;
    bool m_bossSpawned = false;
    bool m_bossDefeated = false;
};

// 7 关配置表
const WaveConfig LEVEL_TABLE[7] = {
    {1, 3, 1.5, 1.0, 0, true, 1},   // 第1关：3波，简单
    {2, 3, 1.4, 1.1, 0, true, 2},   // 第2关：3波
    {3, 4, 1.3, 1.2, 1, true, 2},   // 第3关：4波，中型机出现
    {4, 4, 1.2, 1.3, 1, true, 3},   // 第4关：4波
    {5, 5, 1.1, 1.4, 2, true, 3},   // 第5关：5波，大型机出现
    {6, 5, 1.0, 1.5, 3, true, 4},   // 第6关：5波
    {7, 5, 0.9, 1.8, 5, true, 4},   // 第7关：5波，最难
};
```

---

### PowerUpManager 设计

```cpp
enum class PowerUpType { Hp, Fire, Shield };

class PowerUpManager {
public:
    void update(float dt, const std::vector<Enemy>& enemies,
                std::vector<Bullet>& bullets);
    // 当敌机被击落时概率掉落道具
    void onEnemyDestroyed(const Enemy& enemy,
                          std::vector<Actor>& powerUps);

private:
    static constexpr float DROP_CHANCE = 0.15f;  // 15% 掉落概率
    static constexpr float FIRE_BOOST_DURATION = 15.0f;  // 火力加强持续时间

    PowerUpType randomDrop();  // 随机选择道具类型
};
```

---

### 敌机体系重构

```cpp
// Enemy 基类 — 统一攻击接口
class Enemy {
public:
    virtual ~Enemy() = default;
    virtual void update(float dt);
    virtual bool canAttack(float dt);     // 根据攻击间隔判断
    virtual void attack(std::vector<Bullet>& bullets);  // 发射子弹

    bool isDead() const;
    bool isOffScreen() const;
    virtual int getScore() const;
    // ...
};

// 小型机 — 直线下飞，不攻击
class EnemySmall : public Enemy { /* 继承默认行为 */ };

// 中型机 — 左右摆动，定时单发
class EnemyMedium : public Enemy {
    void update(float dt) override;  // 正弦摆动
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets) override;  // 单发
};

// 大型机 — 缓慢移动，V形散射
class EnemyLarge : public Enemy {
    void update(float dt) override;
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets) override;  // V形双发
};

// 精英机 — 追踪玩家 X 坐标，扇形弹幕
class EnemyElite : public Enemy {
    void update(float dt) override;  // 跟踪玩家
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets) override;  // 3发散弹
};
```

---

### BOSS 完整设计

```cpp
enum class BossPhase { Phase1, Phase2, Phase3 };

enum class AttackType {
    Single,      // 单发直线
    Double,      // V 形双发
    Spread,      // 扇形散弹
    Aimed,       // 瞄准玩家
    Barrage,     // 全屏弹幕（圆形/螺旋扩散）
    Summon       // 召唤小怪
};

struct BossAttackPattern {
    AttackType type;
    float interval;       // 攻击间隔
    int bulletCount;      // 子弹数量
    float spreadAngle;    // 散射角度
    float bulletSpeed;    // 子弹速度
};

class Boss : public Enemy {
    BossPhase m_phase;
    std::vector<BossAttackPattern> m_phasePatterns[3];  // 每阶段攻击模式列表
    float m_attackTimer = 0.0f;
    int m_patternIndex = 0;

    void update(float dt) override {
        updatePhase();     // 血量检测 → 阶段切换
        updateMovement(dt);
        updateAttack(dt);  // 按 pattern 攻击
    }

    void updatePhase() {
        float hpRatio = (float)m_hp / m_maxHp;
        if (hpRatio < 0.25f)      m_phase = BossPhase::Phase3;
        else if (hpRatio < 0.5f)  m_phase = BossPhase::Phase2;
    }
};
```

### 全屏弹幕函数（迭代 3 最终大招）

```cpp
// 圆形弹幕 — 360° 均匀分布
void spawnCircularBarrage(float cx, float cy, int count, float speed,
                          std::vector<Bullet>& bullets) {
    for (int i = 0; i < count; ++i) {
        float angle = 2 * M_PI * i / count;
        bullets.emplace_back(cx, cy,
            std::cos(angle) * speed, std::sin(angle) * speed, Bullet::Enemy);
    }
}

// 螺旋弹幕 — 每帧旋转
void spawnSpiralBarrage(float cx, float cy, int count, float speed,
                        float baseAngle, std::vector<Bullet>& bullets) {
    for (int i = 0; i < count; ++i) {
        float angle = baseAngle + 2 * M_PI * i / count;
        bullets.emplace_back(cx, cy,
            std::cos(angle) * speed, std::sin(angle) * speed, Bullet::Enemy);
    }
}
// 调用方式：每帧 baseAngle += 0.1，实现螺旋旋转效果
```

### GameMapVM 集成

```cpp
// GameMapVM 新增成员
WaveManager m_waveManager;
PowerUpManager m_powerUpManager;
GameMode m_gameMode = GameMode::Campaign;
int m_bossHp = 0, m_bossMaxHp = 0;

// 新增命令
std::function<void()>     getPauseCommand();
std::function<void(int)>  getSelectModeCommand();  // 0=闯关 1=无尽

// 新增属性
int getWave() const noexcept;
int getBossHp() const noexcept;
int getBossMaxHp() const noexcept;
GameMode getGameMode() const noexcept;

// tickImpl 扩展
void tickImpl(float dt) {
    if (m_state != GameState::Playing) return;
    // ... 原有逻辑 ...
    m_waveManager.update(dt, m_enemies, m_player);    // 波次管理
    m_powerUpMgr.update(dt, m_enemies, m_bullets);    // 道具
    // BOSS 血量同步
    syncBossHp();
    emit propertyChanged(PROP_ID_MAP);
}
```

---

## ③ View Agent

**目录**：`src/view/` + `include/view/`

### 需要创建/修改的文件

| 文件 | 变更 |
|---|---|
| `src/view/GameScene.cpp` | 扩展绘制：按 `ActorType` 区分中型机（红色）、大型机（黑色）、BOSS（2x 大图）、道具（颜色图标）；道具用 `drawPixmap` 渲染 |
| `src/view/BossHealthBar.hpp/cpp` | **新建** — BOSS 血条：屏幕顶部红色血条，`setHp(int)/setMaxHp(int)`，仅 BOSS 存活时可见 |
| `src/view/EnemyItem.cpp` | 扩展：支持不同尺寸（BOSS 2x 大小）和不同图片 |
| `src/view/BulletItem.cpp` | 扩展：玩家子弹蓝色，敌方子弹红色 |

### GameView 新增 setter

```cpp
void setWavePtr(const int* p);
void setBossHpPtr(const int* p);
void setBossMaxHpPtr(const int* p);
```

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### 需要修改的文件

| 文件 | 变更 |
|---|---|
| `src/resource/SaveManager.cpp` | 扩展：`saveCampaignProgress(int level)` / `loadCampaignProgress()`；存档 JSON 增加 `campaignLevel` 字段 |

### QRC（迭代 2 已完成，确认资源存在即可）

确认 `resources/resources.qrc` 已包含所有需要的图片 alias。

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### 需要修改的文件

`src/app/AppAgent.cpp`：

```cpp
void AppAgent::init() {
    // ... 迭代 1~2 原有逻辑 ...

    // 桥接变量新增
    m_bridgeWave      = 0;
    m_bridgeBossHp    = 0;
    m_bridgeBossMaxHp = 0;

    // 新增属性绑定
    m_gameView->setWavePtr(&m_bridgeWave);
    m_gameView->setBossHpPtr(&m_bridgeBossHp);
    m_gameView->setBossMaxHpPtr(&m_bridgeBossMaxHp);

    // 命令绑定（已在迭代 2 中完成 setSelectModeCommand / setPauseCommand）

    // App 监听 state 做关卡进度持久化
    // 在 onViewModelChanged 中
    // case PROP_ID_GAME_STATE:
    //     if (...) SaveManager.saveCampaignProgress(level);
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

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `tests/test_wave_manager.cpp` | WaveManager 波次生成、关卡配置表、BOSS 触发、无尽模式难度递增 |
| `tests/test_power_up.cpp` | PowerUpManager 掉落概率分布、拾取效果（回血/火力/护盾） |
| `tests/test_boss.cpp` | BOSS 阶段转换（血量阈值）、攻击模式切换、弹幕生成验证 |

### 现有文件扩展

| 文件 | 新增内容 |
|---|---|
| `tests/test_game_map_vm.cpp` | 新增模式选择、闯关流程、BOSS 战集成 |
| `tests/test_collision.cpp` | 新增敌方子弹 vs 玩家碰撞、道具拾取碰撞 |

---

## 开发顺序

```
① Common (PropertyIds/Wave/BOSS 属性ID + 新枚举)
     ↓
② ViewModel (WaveManager → Enemy/Boss扩展 → PowerUpManager → GameMapVM集成) ← 主力
     ↓
④ Resource (SaveManager 关卡存档)
     ↓
③ View (BossHealthBar → 新绘制逻辑)  ← 可与 Resource 并行
     ↓
⑤ App (组装新绑定)
     ↓
⑥ Test (全流程覆盖)
```
