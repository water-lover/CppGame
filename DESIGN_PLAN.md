# 雷霆战机 (Thunder Fighter) — 完整设计方案

## 0. 理论依据

本架构严格遵循课件中 MVVM 模式的定义。关键引用：

> **"ViewModel 层就是前述改进过的表现层。"** — ViewModel 是核心中介层
> **"View 层和 ViewModel 层之间通过三个绑定来解开耦合：单向属性数据绑定、命令绑定、事件通知绑定。"**
> **"由于游戏类的数据即为可绘制对象，不需要转换，所以取消 Model 层，仅使用 ViewModel 层。"**
> **"可绘制对象有游戏地图和精灵图片两种，所以设计两个 ViewModel 类，分别暴露地图属性和图片属性。"**
> **"使用 std::function 类实现命令模式。"**
> **"增加 viewmodel 子目录。"**

### 三绑定图解

```
ViewModel                               View
─────────                               ────
① 属性绑定: signal + 数据              connect(signal, slot)
② 命令绑定: std::function 成员变量      View 持有命令引用，调用 ()
③ 事件绑定: signal                      connect(signal, slot)
```

---

## 1. 整体架构：6 智能体 × 6 领地

```
项目根目录/
│
├── src/
│   ├── common/          ← ① Common Agent
│   ├── viewmodel/       ← ② ViewModel Agent（取消 Model 层）
│   ├── view/            ← ③ View Agent（纯 C++ QGraphicsView）
│   ├── resource/        ← ④ Resource Agent
│   └── app/             ← ⑤ App Agent
│
├── tests/               ← ⑥ Test Agent
│
├── include/             ← 各 Agent 的头文件（与 src 对应子目录）
├── resources/           ← 图片等静态素材
├── CMakeLists.txt
├── vcpkg.json
├── DESIGN_PLAN.md
└── RULES.md
```

---

## 2. 六个智能体详解

---

### ① Common Agent（公共基础设施层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/common/` + `/include/common/` 下的所有文件 |
| **职责** | 系统最底层。存放所有其他智能体都会用到的基础工具 |
| **越界限制** | **严禁** `#include` 系统里任何其他智能体的文件。它完全独立 |

```
src/common/
├── Types.hpp            ← 枚举：EntityType, GameState, Direction
├── Constants.hpp        ← 常量：SCREEN_WIDTH, SCREEN_HEIGHT, FPS, 各实体尺寸
├── MathUtils.hpp/cpp    ← Vec2 向量运算、距离计算
├── Logger.hpp/cpp       ← 日志打印工具
└── Geometry.hpp/cpp     ← Rect / Circle 碰撞检测基础形状
```

---

### ② ViewModel Agent（核心层，取消 Model 层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/viewmodel/` + `/include/viewmodel/` |
| **职责** | 游戏的全部数据和规则。**没有独立的 Model 层**——因为游戏类数据本身就是可绘制对象，不需要类型转换。ViewModel 聚合数据类对象（Player/Enemy/Bullet），通过 `std::function` 命令和 Qt signal 暴露给 View |
| **ViewModel 类** | **GameMapVM** — 地图滚动、背景、关卡信息<br>**SpriteEntityVM** — 玩家、敌机、子弹、碰撞检测、计分、生命等所有精灵实体 |
| **命令模式** | 使用 `std::function` 实现。命令是 ViewModel 的公开成员变量，View 通过引用调用 |
| **三绑定** | ① **属性绑定**：ViewModel 的 signal → View 的 slot<br>② **命令绑定**：View 调用 ViewModel 的 `std::function` 命令<br>③ **事件绑定**：ViewModel 的 signal → View 切换画面/弹窗 |
| **越界限制** | • 可以访问 Common Agent<br>• **绝对不能**访问 View 层（严禁 include 任何 Qt Widgets 头文件）<br>• **绝对不能** include Resource Agent 的头文件 |

```
src/viewmodel/                        include/viewmodel/
├── SpriteEntityVM.cpp                ├── SpriteEntityVM.hpp
├── GameMapVM.cpp                     ├── GameMapVM.hpp
├── Player.cpp                        ├── Player.hpp
├── Enemy.cpp                         ├── Enemy.hpp
├── Bullet.cpp                        ├── Bullet.hpp
├── CollisionSystem.cpp               ├── CollisionSystem.hpp
├── ScoreManager.cpp                  ├── ScoreManager.hpp
├── PowerUpManager.cpp (后续迭代)     ├── PowerUpManager.hpp (后续迭代)
├── AircraftStats.cpp (后续迭代)      ├── AircraftStats.hpp (后续迭代)
├── SkillSystem.cpp (后续迭代)        ├── SkillSystem.hpp (后续迭代)
└── WaveManager.cpp (后续迭代)        └── WaveManager.hpp (后续迭代)
```

---

### ③ View Agent（视图表现层）— 纯 C++

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/view/` + `/include/view/` |
| **技术方案** | Qt5 `QGraphicsView` + `QGraphicsScene` + 自定义 `QGraphicsItem` 子类 |
| **职责** | 使用 QPainter 渲染所有画面、捕获键盘输入、管理界面切换。**纯 C++，无 QML/JS** |
| **三绑定实现** | ① **属性绑定**：connect ViewModel 的 signal → 更新 Item 位置/文本<br>② **命令绑定**：View 持有 ViewModel 的 std::function 引用，用户操作时调用<br>③ **事件绑定**：connect ViewModel 的 signal → 切换画面 |
| **越界限制** | • 可以访问 Common Agent<br>• 可以 include ViewModel 的头文件（为了 connect 信号和持有命令引用）<br>• **不能** include 游戏数据类的头文件（Player.hpp / Enemy.hpp / Bullet.hpp）<br>• **不能** include Resource Agent 的实现细节 |

```
src/view/                             include/view/
├── GameView.hpp/cpp                  ← QGraphicsView 主窗口 + 场景管理
├── PlayerItem.hpp/cpp                ← 玩家飞机（QGraphicsPixmapItem）
├── EnemyItem.hpp/cpp                 ← 敌机（QGraphicsPixmapItem）
├── BulletItem.hpp/cpp                ← 子弹（QGraphicsPixmapItem）
├── StarFieldItem.hpp/cpp             ← 星空滚动背景（QGraphicsItem 自定义 paint）
├── HudOverlay.hpp/cpp                ← 分数/生命覆盖层（QWidget 叠加）
├── StartScreen.hpp/cpp               ← 开始界面（QWidget 叠加）
├── GameOverScreen.hpp/cpp            ← 游戏结束界面（QWidget 叠加）
├── AircraftSelectScreen.hpp/cpp      ← 战机选择（后续迭代）
├── ModeSelectScreen.hpp/cpp          ← 模式选择（后续迭代）
├── BossHealthBar.hpp/cpp             ← BOSS 血条（后续迭代）
├── PauseOverlay.hpp/cpp              ← 暂停覆盖层（后续迭代）
├── StageClearScreen.hpp/cpp          ← 关卡结算界面（后续迭代）
├── UpgradeScreen.hpp/cpp             ← 无尽模式升级界面（后续迭代）
└── ExplosionEffect.hpp/cpp           ← 爆炸特效（后续迭代）
```

---

### ④ Resource Agent（资源与持久化管理层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/resource/` + `/include/resource/` |
| **职责** | 磁盘 I/O：加载 PNG 图片到 QPixmap、读写最高分 JSON 存档 |
| **越界限制** | • 可以访问 Common Agent<br>• **不可读** ViewModel 的任何文件<br>• **不可读** View 的任何文件 |

```
src/resource/
├── AssetManager.hpp/cpp       ← getImage("playerShip") → QPixmap（带缓存）
└── SaveManager.hpp/cpp        ← loadHighScore() / saveHighScore(int)（JSON）
```

**关键规则**：ViewModel 和 View **不能直接调用 QFile / QImage** 读写文件。所有文件 I/O 必须通过 Resource Agent。

---

### ⑤ App Agent（应用生命周期层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/app/` + `/include/app/` |
| **职责** | main 函数、创建所有 Agent 实例、建立三绑定连接、启动 QTimer 帧循环 |
| **技术方案** | `QApplication` + `QGraphicsView`，**无** QQmlApplicationEngine |
| **越界限制** | 只做组装，**绝对不能**写游戏逻辑或渲染代码 |

```
src/app/
├── AppAgent.hpp/cpp           ← init() / run() / shutdown()
└── main.cpp                   ← 入口
```

---

### ⑥ Test Agent（测试层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/tests/` 目录下的所有文件 |
| **职责** | 单元测试 Common 和 ViewModel 的代码正确性（不启动 Qt 窗口） |
| **越界限制** | 可以访问 Common 和 ViewModel，**不能** include View 的任何文件 |

```
tests/
├── test_main.cpp              ← Catch2 入口
├── test_player.cpp            ← Player 数据类测试
├── test_collision.cpp         ← CollisionSystem 测试
├── test_game_model.cpp        ← SpriteEntityVM 集成测试
└── test_map_vm.cpp            ← GameMapVM 测试（后续）
```

---

## 3. 层间依赖关系

```
┌────────────────────────────────────────────────────────────┐
│  ⑤ App Agent     认识所有人，只组装，不写逻辑                │
│                    在 init() 中建立三绑定连接                │
├────────────────────────────────────────────────────────────┤
│  ③ View Agent     include ViewModel 的头文件               │
│                    connect 信号 + 持有 std::function 引用    │
│                    绝不能直接操作数据类                      │
├────────────────────────────────────────────────────────────┤
│  ② ViewModel      认识 Common，绝对不认识 View              │
│                    （只通过 signal + std::function 暴露）     │
├────────────────────────────────────────────────────────────┤
│  ④ Resource       认识 Common，不认识 ViewModel / View     │
├────────────────────────────────────────────────────────────┤
│  ① Common         不认识任何人，被所有人认识                  │
├────────────────────────────────────────────────────────────┤
│  ⑥ Test           认识 Common + ViewModel                  │
└────────────────────────────────────────────────────────────┘
```

**依赖方向**：`Common ← ViewModel ← View ← App`，上层可调用下层，**下层绝不调用上层**。

### 三绑定详细数据流

```
SpriteEntityVM                           View (QGraphicsView)
──────────────                           ────────────────────

① 属性绑定（ViewModel → View，单向数据流）
─────────────────────────────────────────
  signal playerPosChanged()  ──connect──→  PlayerItem::onPlayerPosChanged()
  signal enemiesChanged()    ──connect──→  GameView::syncEnemyItems()
  signal bulletsChanged()    ──connect──→  GameView::syncBulletItems()
  signal scoreChanged(int)   ──connect──→  HudOverlay::setScore(int)
  signal livesChanged(int)   ──connect──→  HudOverlay::setLives(int)

② 命令绑定（View → ViewModel，操作触发）
─────────────────────────────────────────
  cmdStartGame()       ←──按钮点击───  StartScreen
  cmdMoveUp(bool)      ←──键盘 W/↑──  GameView::keyPressEvent
  cmdMoveDown(bool)    ←──键盘 S/↓──  GameView::keyPressEvent
  cmdMoveLeft(bool)    ←──键盘 A/←──  GameView::keyPressEvent
  cmdMoveRight(bool)   ←──键盘 D/→──  GameView::keyPressEvent
  cmdTick(float)       ←──QTimer(16ms)  GameView::tick()

③ 事件绑定（ViewModel → View，状态通知）
─────────────────────────────────────────
  signal gameStarted()  ──connect──→  GameView::onGameStarted()
  signal gameOver()     ──connect──→  GameView::onGameOver()
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

  QTimer 触发 (16ms)
       │
       ▼
  App Agent:     QTimer::timeout → GameView::tick()
       │
       ▼
  View:          GameView::tick()
       │              ├─ cmdTick(0.016f)   ← 调用 ViewModel 命令
       │
       ▼
  ViewModel:     SpriteEntityVM::tickImpl(dt)
       │              ├─ Player.update(dt)        ← 移动、射击、无敌计时
       │              ├─ Bullet.update(dt)         ← 子弹飞行
       │              ├─ Enemy.update(dt)          ← 敌机 AI 移动
       │              ├─ spawnEnemy()              ← 定时生成敌机
       │              ├─ CollisionSystem.check()   ← 碰撞检测
       │              │    ├─ 子弹 vs 敌机 → 加分
       │              │    └─ 敌机 vs 玩家 → 扣血
       │              ├─ 清理死亡/离屏实体
       │              │
       │              └─ emit scoreChanged(n)
       │              └─ emit livesChanged(n)
       │              └─ emit playerPosChanged()
       │              └─ emit enemiesChanged()
       │              └─ emit bulletsChanged()
       │              └─ if (死亡) emit gameOver()
       │
       ▼
  View:          收到信号 → 更新画面
       │              ├─ HudOverlay::setScore(n)
       │              ├─ HudOverlay::setLives(n)
       │              ├─ PlayerItem 移动到新位置
       │              ├─ 重建 EnemyItem 列表（匹配最新敌人数据）
       │              ├─ 重建 BulletItem 列表
       │              └─ GameOverScreen 弹出
```

同时执行的还有 GameMapVM 的滚动更新：

```
  QTimer::timeout → cmdUpdate(0.016f)
       │
       ▼
  GameMapVM::updateImpl(dt)
       │              └─ scrollOffset += dt * 0.1
       │              └─ emit scrollOffsetChanged(offset)
       ▼
  StarFieldItem 根据 offset 更新星空位置
```

---

## 6. 文件修改权限矩阵

| 文件路径 | 归属 Agent | 谁可以修改 |
|---|---|---|
| `src/common/Types.hpp` | ① Common | **仅** ① |
| `src/viewmodel/Player.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/SpriteEntityVM.cpp` | ② ViewModel | **仅** ② |
| `src/viewmodel/GameMapVM.cpp` | ② ViewModel | **仅** ② |
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
| **ViewModel** | SpriteEntityVM（玩家+敌机+子弹+碰撞+分数+生命），GameMapVM（星空滚动） |
| **View** | GameView + PlayerItem + EnemyItem + BulletItem + StarFieldItem + StartScreen + HudOverlay + GameOverScreen |
| **渲染** | **纯 C++** QGraphicsView + QGraphicsItem，无 QML |
| **操作** | WASD 移动 + 自动开火 |
| **命令** | cmdStartGame / cmdMoveUp/Down/Left/Right / cmdTick — 全部 std::function |
| **存档** | 最高分读写（SaveManager） |

**可交付判断**：打开游戏 → 开始 → WASD移动 → 敌机出现 → 自动射击击落敌机 → 撞敌机扣血 → 命归零 → Game Over → 再来一局

---

### 迭代 2 — BOSS 战 + 7 关闯关 🏆

新增：中型/大型敌机、BOSS AI、闯关模式(7关波次表)、道具掉落、模式选择界面、BOSS 血条。新增 WaveManager、PowerUpManager。

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
│  自动开火 + WASD 移动 + Space 放技能                        │
│       ↓                                                    │
│  击落敌机 → 得分 + 概率掉落道具                             │
│       ↓                                                    │
│  击败 BOSS → 通关 (闯关) / 继续 (无尽)                      │
│       ↓                                                    │
│  无尽模式：用星核碎片升级战机 → 挑战更高波次                  │
└────────────────────────────────────────────────────────────┘
```
