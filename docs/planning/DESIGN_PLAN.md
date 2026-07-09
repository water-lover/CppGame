# 雷霆战机 (Thunder Fighter) — 完整设计方案

## 0. 理论依据

本架构严格遵循课件中 MVVM/MVFM 模式的定义。核心引用：

> **"ViewModel 层就是前述改进过的表现层。"** — ViewModel 是核心中介层
> **"View 层和 ViewModel 层之间通过三个绑定来解开耦合：单向属性数据绑定、命令绑定、事件通知绑定。"**
> **"由于游戏类的数据即为可绘制对象，不需要转换，所以取消 Model 层，仅使用 ViewModel 层。"**
> **"可绘制对象有游戏地图和精灵图片两种，所以设计两个 ViewModel 类，分别暴露地图属性和图片属性。"**
> **"使用 std::function 类实现命令模式。"**
> **"增加 viewmodel 子目录。"**

### 三绑定定义

```
① 属性绑定（ViewModel → View）: View 持有 const T* 指针，只读访问 ViewModel 的数据
② 命令绑定（View → ViewModel）: View 持有 std::function，调用时由 App 注入
③ 事件绑定（ViewModel → View）: ViewModel 发出通知，App 中转给 View
```

> **📌 PropertyTrigger 等价说明**：老师 ex5 中 ViewModel 继承 `PropertyTrigger`，通过 `fire(PROP_ID_MAP)` 发出通知，View 通过 `add_notification(callback)` 注册回调。<br>> 在 Qt 版本中：ViewModel 继承 `QObject`，通过 `emit propertyChanged(PROP_ID_MAP)` 发出通知，App 通过 `connect()` 将信号连接到 View 的槽函数。<br>> **两种方式的架构意义完全相同**——ViewModel 发出通知时不知道接收者是谁，View 在不知道 ViewModel 具体类的情况下接收通知。

---

## 1. 整体架构：6 智能体 × 6 领地

```
项目根目录/
│
├── src/
│   ├── common/          ← ① Common Agent
│   ├── viewmodel/       ← ② ViewModel Agent（核心，聚合数据类）
│   ├── view/            ← ③ View Agent（纯 C++ QGraphicsView，无 QML）
│   ├── resource/        ← ④ Resource Agent
│   └── app/             ← ⑤ App Agent（唯一组装者）
│
├── tests/               ← ⑥ Test Agent
│
├── include/
│   ├── common/          ← ①
│   ├── viewmodel/       ← ②
│   ├── view/            ← ③
│   ├── resource/        ← ④
│   └── app/             ← ⑤
│
├── resources/           ← 图片等静态素材（由 Resource Agent 加载）
├── CMakeLists.txt
├── vcpkg.json
├── docs/
│   └── planning/
│       ├── DESIGN_PLAN.md     ← 本文件
│       ├── RULES.md
│       └── iter1_instructions.md
└── ...
```

---

## 2. 六个智能体详解

---

### ① Common Agent（公共基础设施层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `src/common/` + `include/common/` |
| **职责** | 系统最底层。所有其他智能体都会用到的基础工具 |
| **越界限制** | **严禁** `#include` 系统里任何其他智能体的文件。完全独立 |

```
src/common/                         include/common/
├── Types.hpp                       ├── Types.hpp          ← EntityType, GameState, Direction
├── Constants.hpp                   ├── Constants.hpp      ← SCREEN_WIDTH, PLAYER_SPEED, ...
├── MathUtils.cpp                   ├── MathUtils.hpp      ← Vec2, distance, normalize
├── Logger.cpp                      ├── Logger.hpp         ← log(tag, msg)
├── Geometry.cpp                    ├── Geometry.hpp       ← Rect, Circle, overlaps
├── Actor.hpp                       ├── Actor.hpp          ← 精灵数据结构（type, x, y, hp）
├── AirMap.cpp                      ├── AirMap.hpp         ← 精灵集合容器（可被 View 遍历绘制）
└── PropertyIds.hpp                 └── PropertyIds.hpp    ← PROP_ID_MAP, PROP_ID_SCORE, ...
```

**像素质量与屏幕适应性：**

本游戏采用**逻辑分辨率 800×600** 作为设计基准，所有游戏逻辑（碰撞检测、位置计算）基于此分辨率。

| 维度 | 方案 |
|---|---|
| **像素质量** | 替换 Kenney 低分辨率素材为高分辨率（2x/3x）图片；View 层渲染时按逻辑分辨率缩放绘制，素材本身采用更高清的源文件 |
| **屏幕适应** | 去掉 `setFixedSize()`；用 `fitInView()` 保持宽高比缩放；支持窗口拉伸和全屏模式（F11 切换）；HUD 和界面元素按比例缩放 |
| **高分屏支持** | 启用 `Qt::HighDpiScaleFactorRoundingPolicy`，确保 4K 屏幕不模糊 |

**Actor.hpp + AirMap.hpp** — 这是 View 唯一能看到的数据结构（对齐老师的 `air_map.h`）。View 通过 `const AirMap*` 指针读取所有精灵的位置和类型来进行绘制，但完全不知道这些数据来自哪些数据类。

```cpp
// Actor.hpp — View 和 ViewModel 都 include（在 common/ 下）
enum class ActorType {
    Player, EnemySmall, EnemyMedium, EnemyLarge,
    PlayerBullet, EnemyBullet, PowerUp, Boss
};

struct Actor {
    ActorType type;
    float x, y;         // 逻辑坐标（对齐老师 ex5 的 int 像素坐标）
    int hp;             // 当前生命值（用于血条显示）
    int maxHp;          // 最大生命值
};

// AirMap.hpp — 精灵集合，View 遍历此集合绘制所有对象
class AirMap {
public:
    void clear();
    size_t size() const;
    const Actor& getAt(size_t idx) const;
    Actor& getAt(size_t idx);
    void append(const Actor& actor);
    // ...
};
```

**PropertyIds.hpp** 是事件通知的标识枚举，采用老师课件中的方式——"使用枚举值是较好的方式，整个工程需要的数据名字枚举值可以统一定义在一个头文件中"。

```cpp
// PropertyIds.hpp 示例
enum {
    PROP_ID_MAP = 1,       // 游戏地图（所有精灵位置）变化
    PROP_ID_SCORE,         // 分数变化
    PROP_ID_LIVES,         // 生命变化
    PROP_ID_GAME_STATE,    // 游戏状态变化（Playing / GameOver / ...）
    PROP_ID_MAP_OFFSET,    // 地图滚动偏移变化
};
```

---

### ② ViewModel Agent — 核心层

| 项目 | 内容 |
|---|---|
| **负责文件** | `src/viewmodel/` + `include/viewmodel/` |
| **职责** | **游戏的全部数据和规则。** 采用 MVFM 思想，拆分两个 ViewModel 类。<br>• **GameMapVM (Function Model)** — 地图/精灵位置/碰撞/计分/生命/波次，偏向数据<br>• **SpiritVM (ViewModel)** — 精灵图片资源，偏向界面 |
| **命令模式** | 命令通过 `std::function` 实现，ViewModel 提供 getter 方法返回命令，由 App 注入给 View |
| **事件通知** | 数据变化时调用 `fire(PROP_ID_XXX)`，由 PropertyTrigger（Qt 中可用 QObject + signal 替代）发出 |
| **属性暴露** | 通过 getter 方法返回 `const T*` 指针，View 只读访问 |
| **可读** | ✅ 可读 Common Agent<br>❌ **不可读** View 的任何文件<br>❌ **不可读** Resource 的实现细节（但可通过接口调用 SaveManager） |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | `src/view/` — **绝对不能** include 任何 UI 头文件<br>`src/resource/` — 不直接写文件，通过 Resource 接口调用 |
| **Qt 限制** | 可以 `#include <QObject>` 用于信号机制，**不能** include 任何 Widgets/Quick 头文件 |

```
src/viewmodel/                      include/viewmodel/
├── GameMapVM.cpp                   ├── GameMapVM.hpp     ← 游戏地图 ViewModel（核心 FM）
├── SpiritVM.cpp                    ├── SpiritVM.hpp      ← 精灵图片 ViewModel
├── Player.cpp                      ├── Player.hpp        ← 玩家数据类
├── Enemy.cpp                       ├── Enemy.hpp         ← 敌机数据类
├── Bullet.cpp                      ├── Bullet.hpp        ← 子弹数据类
├── CollisionSystem.cpp             ├── CollisionSystem.hpp ← 碰撞检测工具
├── ScoreManager.cpp                ├── ScoreManager.hpp  ← 计分 + 最高分
├── PowerUpManager.cpp (后续迭代)   ├── PowerUpManager.hpp
├── AircraftStats.cpp (后续迭代)    ├── AircraftStats.hpp
├── SkillSystem.cpp (后续迭代)      ├── SkillSystem.hpp
└── WaveManager.cpp (后续迭代)      └── WaveManager.hpp
```

**GameMapVM 的核心结构（对齐 ex5 Plane 的 GameViewModel）：**

```cpp
class GameMapVM : public QObject {
    Q_OBJECT
public:
    explicit GameMapVM(QObject* parent = nullptr);

    // ① 属性暴露 — 返回 const 指针，View 只读
    const AirMap* getMap() const noexcept { return &m_map; }
    // ... 其他属性

    // ② 命令暴露 — 返回 std::function，由 App 注入给 View
    std::function<void(int)> getMoveUpCommand();
    std::function<void(int)> getMoveDownCommand();
    std::function<void(int)> getMoveLeftCommand();
    std::function<void(int)> getMoveRightCommand();
    std::function<void(float)> getTickCommand();

signals:
    // ③ 事件通知 — Qt signal 替代 PropertyTrigger::fire
    void propertyChanged(uint32_t propertyId);

private:
    AirMap m_map;
    // ... 其他数据成员

    // 命令实现
    void moveUpImpl(int);  // 由 std::function lambda 调用
    void tickImpl(float);
};
```

**SpiritVM 的核心结构（对齐 ex5 Plane 的 SpiritViewModel）：**

```cpp
class SpiritVM {
public:
    // 属性暴露 — 返回精灵图片指针
    const QPixmap* getPlayerPixmap() const noexcept;
    const QPixmap* getEnemySmallPixmap() const noexcept;
    // ...

    bool initialize();  // 加载图片
    // ...
};
```

---

### ③ View Agent — 纯 C++（无 QML）

| 项目 | 内容 |
|---|---|
| **负责文件** | `src/view/` + `include/view/` |
| **技术方案** | Qt5 `QGraphicsView` + `QGraphicsScene` + 自定义 `QGraphicsItem` 子类 |
| **职责** | 使用 QPainter 渲染所有画面、捕获键盘输入、管理界面切换、管理帧循环 QTimer。<br>**纯 C++，无 QML/JS**。对齐老师 ex5：MainWindow 自带 timeout 回调驱动帧循环 |
| **关键约束** | **绝对不能 `#include` 任何 ViewModel 的头文件！**<br>View **不**认识 ViewModel，只认识：<br>① `const T*` 属性指针（来自 common/ 的数据类型）<br>② `std::function` 命令（由 App 注入）<br>③ `uint32_t` 属性 ID（收到通知后判断哪个属性变了）<br><br>**屏幕适应性**：View 层负责所有坐标转换。游戏逻辑坐标 800×600，View 通过 `fitInView()` 自动缩放适配实际窗口大小。HUD/菜单等覆盖层按百分比定位，不依赖绝对像素。 |
| **可读** | ✅ 可读 Common Agent（包含数据类型头文件）<br>✅ **可读** ViewModel 的公开头文件？ **不可以！**<br>❌ **不可读** ViewModel 的任何头文件<br>❌ **不可读** Resource 的实现细节 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ❌ **不能** include `viewmodel/` 下的任何文件（包括 Player.hpp 等数据类）<br>❌ **不能**直接操作游戏数据<br>❌ **不能**直接读写文件 |

> ⚠️ **重要**：View 只从 App 处接收 `const T*` 指针和 `std::function` 对象，完全不知道这些对象来自哪个 ViewModel 类。这就是课件所说的"View 层和 ViewModel 层彻底解开了耦合"。

**GameView 自带 QTimer（对齐 ex5 MainWindow 的 Fl::add_timeout）：**
```cpp
// GameView 构造函数
GameView::GameView(QWidget* parent) : QGraphicsView(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_tickCommand) m_tickCommand(0.016f);
    });
    m_timer->start(16);  // ~60 FPS
}
```

```
src/view/                           include/view/
├── GameView.cpp                    ├── GameView.hpp        ← QGraphicsView 主窗口
├── GameScene.cpp                   ├── GameScene.hpp       ← QGraphicsScene 场景
├── PlayerItem.cpp                  ├── PlayerItem.hpp      ← 玩家飞机（QGraphicsPixmapItem）
├── EnemyItem.cpp                   ├── EnemyItem.hpp       ← 敌机（QGraphicsPixmapItem）
├── BulletItem.cpp                  ├── BulletItem.hpp      ← 子弹（QGraphicsPixmapItem）
├── StarFieldItem.cpp               ├── StarFieldItem.hpp   ← 星空滚动背景
├── HudOverlay.cpp                  ├── HudOverlay.hpp      ← 分数/生命覆盖层
├── StartScreen.cpp                 ├── StartScreen.hpp     ← 开始界面
├── GameOverScreen.cpp              ├── GameOverScreen.hpp  ← 游戏结束界面
├── AircraftSelectScreen.cpp        ├── AircraftSelectScreen.hpp  (后续迭代)
├── ModeSelectScreen.cpp            ├── ModeSelectScreen.hpp      (后续迭代)
├── BossHealthBar.cpp               ├── BossHealthBar.hpp         (后续迭代)
├── PauseOverlay.cpp                ├── PauseOverlay.hpp          (后续迭代)
├── StageClearScreen.cpp            ├── StageClearScreen.hpp      (后续迭代)
├── UpgradeScreen.cpp               ├── UpgradeScreen.hpp         (后续迭代)
└── ExplosionEffect.cpp             └── ExplosionEffect.hpp       (后续迭代)
```

---

### ④ Resource Agent（资源与持久化管理层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `src/resource/` + `include/resource/` |
| **职责** | 磁盘 I/O：加载 PNG 图片到 QPixmap（带缓存）、读写最高分/升级数据 JSON 存档 |
| **越界限制** | • 可读 Common Agent<br>• **不可读** ViewModel 的任何文件<br>• **不可读** View 的任何文件 |

```
src/resource/                       include/resource/
├── AssetManager.cpp                ├── AssetManager.hpp  ← getImage(key) → QPixmap
└── SaveManager.cpp                 └── SaveManager.hpp   ← load/saveHighScore(int)
```

**关键规则**：ViewModel 和 View **绝对不能直接调用 QFile / QImage / QPixmap 读写文件**。所有文件 I/O 必须通过 Resource Agent。

---

### ⑤ App Agent（应用生命周期层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `src/app/` + `include/app/` |
| **职责** | main 函数、创建所有 Agent 实例、**建立三绑定连接**（对齐老师 ex5 AirApp）<br>帧循环由 View 层的 GameView 自行管理（对齐 ex5 的 MainWindow 自带 timeout 回调） |
| **技术方案** | `QApplication` + `QGraphicsView`，**无** QQmlApplicationEngine |
| **可读** | ✅ 可读所有 Agent |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ⚠️ **绝对不能**写游戏逻辑或渲染代码——只做组装和连接 |

```
src/app/                            include/app/
├── AppAgent.cpp                    ├── AppAgent.hpp  ← init() / run() / shutdown()
└── main.cpp                        └── (无头文件)
```

**AppAgent 的组装流程（对齐 ex5/ex6 AirApp）：**

```cpp
class AppAgent {
public:
    bool initialize() {
        // 1. 创建所有 Agent
        // 2. ① 属性绑定 — 将 ViewModel 的数据指针注入 View
        m_gameScene.setMap(m_gameMapVM.getMap());
        m_gameScene.setPlayerPixmap(m_spiritVM.getPlayerPixmap());

        // 3. ② 命令绑定 — 将 ViewModel 的命令注入 View
        m_gameView.setTickCommand(m_gameMapVM.getTickCommand());
        m_gameView.setMoveUpCommand(m_gameMapVM.getMoveUpCommand());

        // 4. ③ 事件绑定 — ViewModel 的通知转发给 View
        // Qt 方式:
        connect(&m_gameMapVM, &GameMapVM::propertyChanged,
                &m_gameView, &GameView::onPropertyChanged);
        // 或 lambda:
        connect(&m_gameMapVM, &GameMapVM::propertyChanged,
                [this](uint32_t id) { m_gameView.onPropertyChanged(id); });

        // ⚠️ 注意：不在这里启动帧循环！
        // 对齐老师 ex5：GameView 自带的 QTimer 管理帧循环
        // 见 GameView 的构造函数
    }
};
```

---

### ⑥ Test Agent（测试层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `tests/` 目录下的所有文件 |
| **职责** | 单元测试 Common 和 ViewModel 的代码正确性（不启动 Qt 窗口） |
| **越界限制** | 可读 Common 和 ViewModel，**不能** include View 的任何文件 |

```
tests/
├── test_main.cpp              ← Catch2 入口（Catch2WithMain 已提供 main）
├── test_player.cpp            ← Player 移动/生命/无敌计时
├── test_collision.cpp         ← CollisionSystem 碰撞检测
├── test_game_map_vm.cpp       ← GameMapVM 集成测试
└── test_spirit_vm.cpp         ← SpiritVM 测试（后续）
```

---

## 3. 层间依赖关系

```
┌──────────────────────────────────────────────────────────────────┐
│  ⑤ App Agent   认识所有人，只组装，不写逻辑                        │
│                 在 init() 中完成三绑定的连接                       │
├──────────────────────────────────────────────────────────────────┤
│  ③ View Agent  绝不认识 ViewModel ❌                             │
│                 只持有：const T* 属性指针（来自 common/）           │
│                        std::function 命令（由 App 注入）           │
│                        通知回调中处理 uint32_t 属性 ID             │
├──────────────────────────────────────────────────────────────────┤
│  ② ViewModel    认识 Common，绝对不认识 View                      │
│                  （只通过 const T* + std::function + signal 暴露）  │
├──────────────────────────────────────────────────────────────────┤
│  ④ Resource     认识 Common，不认识 ViewModel / View             │
├──────────────────────────────────────────────────────────────────┤
│  ① Common       不认识任何人，被所有人认识                          │
├──────────────────────────────────────────────────────────────────┤
│  ⑥ Test         认识 Common + ViewModel                          │
└──────────────────────────────────────────────────────────────────┘
```

**依赖方向**：`Common ← ViewModel ← App → View`，App 是唯一的交汇点。

**关键规则**：View 和 ViewModel 之间**没有直接的头文件依赖**。所有通信通过三种方式：
- `const T*` 指针（数据从 ViewModel 流向 View）
- `std::function` 命令（操作从 View 流向 ViewModel）
- 事件通知（通知从 ViewModel 流向 View）

---

### 三绑定详细数据流

```
GameMapVM (ViewModel)                       GameView / GameScene (View)
─────────────────────                       ──────────────────────────

① 属性绑定（单向数据流，const T* 指针）
────────────────────────────────────────────
  const AirMap* getMap()       ──────────→  const AirMap* m_pMap
  (GameMapVM 拥有数据)                       (View 只读，在 paint() 中使用)

② 命令绑定（View → ViewModel，std::function）
────────────────────────────────────────────
  getMoveUpCommand() 返回        ←────────  std::function<void(int)> m_moveUpCmd
  std::function<void(int)>                   (键盘 W/↑ → 调用命令)
  ↓ 内部调用
  moveUpImpl(type) → 修改数据 → emit propertyChanged(PROP_ID_MAP)

③ 事件绑定（ViewModel → View，signal → slot）
────────────────────────────────────────────
  GameMapVM 发出信号            ──────────→  GameView::onPropertyChanged(uint32_t id)
  emit propertyChanged(id)                    → switch(id) {
                                                case PROP_ID_MAP:  scene()->update(); break;
                                                case PROP_ID_SCORE: hud->updateScore(); break;
                                                ...
                                              }
```

---

## 4. 游戏整体设计

### 4.1 游戏流程

```
启动游戏
    │
    ▼
┌──────────────┐      ┌──────────────────┐
│ 战机选择界面  │ ──→ │ 模式选择界面      │
│ (5选1)       │      │ (闯关 / 无尽)     │
└──────────────┘      └──────┬───────────┘
                             │
              ┌──────────────┴──────────────┐
              │                             │
              ▼                             ▼
    ┌──────────────────┐       ┌──────────────────┐
    │ 闯关模式 (7关)    │       │ 无尽模式          │
    │ 第1关 → ... →    │       │ 小怪波次 → BOSS   │
    │ 第7关 BOSS       │       │ → 继续 → 更强BOSS │
    │ 通关 → 结算评分   │       │ → ... (无限循环)  │
    └──────────────────┘       └──────────────────┘
              │                             │
              │                      ┌──────┴──────┐
              │                      │ 升级界面      │
              │                      │ (用星核碎片   │
              │                      │  强化战机)    │
              │                      └──────┬──────┘
              └──────────┬──────────────────┘
                         ▼
                   ┌──────────┐
                   │ 返回选择  │
                   └──────────┘
```

### 4.2 五架战机

| 编号 | 名称 | 火力 | 生命 | 速度 | 主动技能 | 效果 |
|---|---|---|---|---|---|---|
| ① | **雷霆号** | ★★★ | ★★★ | ★★★ | 雷暴领域 | 全屏雷击 |
| ② | **烈焰号** | ★★★★★ | ★★ | ★★ | 火焰风暴 | 扇形火焰喷射 |
| ③ | **冰霜号** | ★★ | ★★★★★ | ★★ | 极寒护盾 | 护盾+冻结敌机 |
| ④ | **幻影号** | ★★★ | ★★ | ★★★★★ | 时空闪避 | 向前冲刺攻击 |
| ⑤ | **堡垒号** | ★★ | ★★★★ | ★ | 铁壁阵 | 无敌+反弹子弹 |

### 4.3 闯关模式（7 关）

| 关卡 | 主题 | 敌机 | BOSS | 难度 |
|---|---|---|---|---|
| 1 | 初入战场 | 小型机 | 轻型BOSS | ★☆☆ |
| 2 | 空中走廊 | 小型+中型 | 中型BOSS | ★★☆ |
| 3 | 雷云风暴 | 中型为主 | 雷电BOSS | ★★☆ |
| 4 | 敌军要塞 | 中型+大型 | 重型BOSS | ★★★ |
| 5 | 暗夜突袭 | 大量小型+精英 | 隐形BOSS | ★★★ |
| 6 | 火力封锁 | 全类型高密度 | 装甲BOSS | ★★★★ |
| 7 | 最终决战 | 最强配置 | 终极BOSS | ★★★★★ |

每一关流程：`小怪波次(3~5波) → BOSS 出现 → 击败 BOSS → 过关结算`

### 4.4 无尽模式

| 项目 | 内容 |
|---|---|
| **玩法** | 无限循环：小怪波次 → BOSS → 小怪波次(更难) → BOSS(更强) → ... |
| **资源系统** | 击败敌机/BOSS 掉落"星核碎片"，用于永久升级战机 |
| **升级项目** | 火力等级、生命上限、移动速度、技能冷却缩减、子弹伤害 |
| **难度递增** | 每通过一轮，敌机生成速度 +10%，敌机生命 +15% |
| **保存** | 每次升级后存档到本地，下次进游戏继承 |

### 4.5 道具系统（战斗中掉落）

| 道具 | 外观 | 效果 |
|---|---|---|
| ❤️ 回血包 | 红色十字 | 恢复 1 格生命 |
| ⚡ 火力加强 | 蓝色闪电 | 武器等级 +1（持续 15 秒） |
| 🛡️ 护盾 | 金色盾牌 | 获得一次免伤护盾 |
| ⭐ 星核碎片 | 紫色星星 | 无尽模式专属，用于升级 |

### 4.6 操作方式

| 按键 | 功能 |
|---|---|
| **W / ↑** | 向上移动 |
| **A / ←** | 向左移动 |
| **S / ↓** | 向下移动 |
| **D / →** | 向右移动 |
| **Space** | 释放主动技能（有冷却时间） |
| **Esc** | 暂停 |
| **Enter** | 确认/开始 |

### 4.7 玩家属性

| 属性 | 初始值 | 可升级 |
|---|---|---|
| 生命 | 3 | ✅ 每级 +1 上限 |
| 移动速度 | 归一化 0.6/秒 | ✅ 每级 +0.05 |
| 武器等级 | 1（最高 5） | ✅ 基础火力可永久升级 |
| 射击间隔 | 0.2秒 | ✅ 冷却缩减可升级 |
| 技能冷却 | 15~25秒（战机不同） | ✅ 冷却缩减可升级 |

### 4.8 敌机类型

| 类型 | 生命 | 速度 | 分数 | 说明 |
|---|---|---|---|---|
| 小型机 | 1 | 0.2~0.4 | 10 | 直线下飞 |
| 中型机 | 2 | 0.15~0.3 | 30 | 左右摆动 |
| 大型机 | 5 | 0.1~0.2 | 50 | 会发射子弹 |
| BOSS | 50~200 | 0.05~0.1 | 500+ | 多种攻击模式 + 阶段转换 |

---

## 5. 数据流：完整游戏循环

```
每一帧 (60fps):

  GameView 内部 QTimer 触发 (16ms)
       │
       ▼
  View:          GameView::tick()
       │              ├─ m_tickCommand(0.016f)   ← 调用 ViewModel 命令（std::function）
       │
       ▼
  ViewModel:     GameMapVM::tickImpl(dt)
       │              ├─ Player.update(dt)        ← 移动、射击、无敌计时
       │              ├─ Bullet.update(dt)         ← 子弹飞行
       │              ├─ Enemy.update(dt)          ← 敌机 AI 移动
       │              ├─ spawnEnemy()              ← 定时生成敌机
       │              ├─ CollisionSystem.check()   ← 碰撞检测
       │              │    ├─ 子弹 vs 敌机 → 加分
       │              │    └─ 敌机 vs 玩家 → 扣血
       │              ├─ 清理死亡/离屏实体
       │              │
       │              └─ emit propertyChanged(PROP_ID_SCORE)
       │              └─ emit propertyChanged(PROP_ID_LIVES)
       │              └─ emit propertyChanged(PROP_ID_MAP)
       │              └─ if (死亡) emit propertyChanged(PROP_ID_GAME_STATE)
       │
       ▼
  View:          GameView::onPropertyChanged(id)
       │              ├─ case PROP_ID_MAP:    scene()->update()（重绘所有精灵）
       │              ├─ case PROP_ID_SCORE:  hud->updateScore()
       │              ├─ case PROP_ID_LIVES:  hud->updateLives()
       │              └─ case PROP_ID_GAME_STATE: 弹出 GameOverScreen
       │
       ▼
  View:          GameScene::paint() / QGraphicsItem::paint()
       │              ├─ 从 m_pMap 指针读取所有精灵位置
       │              ├─ 用 m_pPlayerPixmap 等绘制图片
       │              └─ 渲染到屏幕
```

同时执行的还有地图滚动：

```
  GameView 内部 QTimer 触发
       │
       ▼
  View:          m_scrollCommand(0.016f)
       │
       ▼
  GameMapVM 更新 scrollOffset
       │
       └─ emit propertyChanged(PROP_ID_MAP_OFFSET)
       │
       ▼
  StarFieldItem 根据偏移更新星空位置
```

---

## 6. 文件修改权限矩阵

| 文件路径 | 归属 Agent | 谁可以修改 |
|---|---|---|
| `include/common/PropertyIds.hpp` | ① Common | **仅** ① |
| `include/common/Constants.hpp` | ① Common | **仅** ① |
| `include/common/Types.hpp` | ① Common | **仅** ① |
| `src/viewmodel/GameMapVM.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/SpiritVM.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/Player.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/Enemy.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/CollisionSystem.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/ScoreManager.cpp` | ② ViewModel | **仅** ② |
| `src/view/GameView.cpp` | ③ View | **仅** ③ |
| `src/view/PlayerItem.cpp` | ③ View | **仅** ③ |
| `src/view/EnemyItem.cpp` | ③ View | **仅** ③ |
| `src/view/HudOverlay.cpp` | ③ View | **仅** ③ |
| `src/view/StartScreen.cpp` | ③ View | **仅** ③ |
| `src/resource/AssetManager.cpp` | ④ Resource | **仅** ④ |
| `src/resource/SaveManager.cpp` | ④ Resource | **仅** ④ |
| `src/app/AppAgent.cpp` | ⑤ App | **仅** ⑤ |
| `tests/test_player.cpp` | ⑥ Test | **仅** ⑥ |
| `CMakeLists.txt` | 所有 Agent | 加新文件时对应 Agent 改 |

---

## 7. 图片资源映射表

### 7.1 玩家战机

| 文件 | 对应战机 |
|---|---|
| `resources/images/MyAircraft/Firepower.png` | ① 雷霆号 |
| `resources/images/MyAircraft/Flame.png` | ② 烈焰号 |
| `resources/images/MyAircraft/Frost.png` | ③ 冰霜号 |
| `resources/images/MyAircraft/Speed.png` | ④ 幻影号 |
| `resources/images/MyAircraft/Defense.png` | ⑤ 堡垒号 |

### 7.2 敌机

| 角色 | 文件 |
|---|---|
| 小型敌机 | `images/PNG/Enemies/enemyRed3.png` |
| 中型敌机 | `images/PNG/Enemies/enemyBlack1.png` |
| 大型敌机 | `images/PNG/Enemies/enemyBlack5.png` |
| BOSS（第7关） | `images/PNG/Sprites/Ships/spaceShips_009.png` |
| BOSS（第4~6关） | `images/PNG/Sprites/Rockets/spaceRockets_001.png` |
| BOSS（第2~3关） | `images/PNG/Sprites/Ships/spaceShips_004.png` |

### 7.3 子弹

| 角色 | 文件 |
|---|---|
| 玩家子弹 | `images/PNG/Lasers/laserBlue16.png` |
| 敌方子弹 | `images/PNG/Lasers/laserRed01.png` |
| 敌方导弹 | `images/PNG/Sprites/Missiles/spaceMissiles_001.png` |

### 7.4 特效

| 效果 | 文件 |
|---|---|
| 爆炸动画 | `images/PNG/Effects/fire00.png` ~ `fire19.png` |
| 引擎火焰 | `images/PNG/Effects/engine1.png` ~ `engine5.png` |
| 护盾 | `images/PNG/Effects/shield1~3.png` |
| 星星粒子 | `images/PNG/Effects/star1~3.png` |

### 7.5 道具

| 道具 | 文件 |
|---|---|
| ❤️ 回血包 | `images/PNG/Power-ups/pill_red.png` |
| ⚡ 火力加强 | `images/PNG/Power-ups/powerupBlue_bolt.png` |
| 🛡️ 护盾 | `images/PNG/Power-ups/shield_gold.png` |
| ⭐ 星核碎片 | `images/PNG/Power-ups/star_gold.png` |

### 7.6 背景

| 用途 | 文件 |
|---|---|
| 游戏背景 | `images/Backgrounds/darkPurple.png` |

---

## 8. 迭代重构规划

### 迭代 1 — MVP 最小可玩版 🎯

**目标**：能"飞起来、打出去、撞上去、死掉重来"的最简闭环。

| 维度 | 内容 |
|---|---|
| **ViewModel** | GameMapVM（玩家/敌机/子弹/碰撞/分数/生命）+ SpiritVM（精灵图片） |
| **View** | GameView + GameScene + PlayerItem + EnemyItem + BulletItem + StarFieldItem + StartScreen + HudOverlay + GameOverScreen |
| **渲染** | **纯 C++** QGraphicsView + QGraphicsItem，无 QML |
| **操作** | WASD 移动 + 自动开火 |
| **命令** | getTickCommand / getMoveUpCommand / getMoveDownCommand / getMoveLeftCommand / getMoveRightCommand / getStartGameCommand — 全部 std::function |
| **存档** | 最高分读写（SaveManager） |

**可交付判断**：打开游戏 → 开始 → WASD移动 → 敌机出现 → 自动射击击落敌机 → 撞敌机扣血 → 命归零 → Game Over → 再来一局

**涉及 Agent**：Common(PropertyIds/Constants) → ViewModel(GameMapVM/Player/Enemy/Bullet/CollisionSystem/ScoreManager + SpiritVM) → View(所有 Item 和界面) → Resource(AssetManager/SaveManager) → App(AppAgent)

---

### 迭代 2 — 高画质 + BOSS 战 + 7 关闯关 🏆

**像素提升：**
- 替换 Kenney 低分辨率素材为高分辨率 PNG（2x 放大 + 抗锯齿）
- 开启 `SmoothPixmapTransform` 渲染缩放
- 替换 5 架战机为自制高分辨率素材

**屏幕适应：**
- 去掉 `setFixedSize()`，支持任意窗口大小
- GameView 的 `resizeEvent` 中调用 `fitInView(0, 0, 800, 600, Qt::KeepAspectRatio)`
- HUD 覆盖层按百分比布局（左上角分数、右上角生命）
- 全屏模式（F11 切换）
- 最小窗口限制 600×450

**新功能：**
- 中型/大型敌机、BOSS AI、闯关模式(7关波次表)、道具掉落
- 模式选择界面、BOSS 血条、PauseOverlay
- 新增 WaveManager、PowerUpManager 到 ViewModel 层
- 新增 ModeSelectScreen、BossHealthBar 到 View 层

### 迭代 3 — 5 战机 + 技能系统 🚀

新增：AircraftStats（5架属性模板）、SkillSystem（5种主动技能）、战机选择界面、HUD 技能CD显示、Space 放技能。

### 迭代 4 — 无尽模式 + 升级系统 🔄

新增：星核碎片掉落、升级界面（火力/生命/速度/冷却）、难度递增曲线、升级数据持久化。

### 迭代 5 — 画面精修 + 平衡调整 ✨

特效增强（爆炸粒子/引擎火焰）、数值平衡、Bug 修复、性能优化、最终版提交。

---

## 9. 核心循环总结

```
┌────────────────────────────────────────────────────────────┐
│                    雷霆战机 核心循环                        │
│                                                            │
│  战机选择 → 模式选择 → 进入战斗                              │
│       ↓                                                    │
│  WASD 移动（命令绑定） + 自动开火                            │
│       ↓                                                    │
│  击落敌机（碰撞检测） → 加分（属性绑定更新 HUD）              │
│       ↓                                                    │
│  击败 BOSS → 通关 (闯关) / 继续 (无尽)                      │
│       ↓                                                    │
│  无尽模式：用星核碎片升级战机 → 挑战更高波次                  │
└────────────────────────────────────────────────────────────┘
```
