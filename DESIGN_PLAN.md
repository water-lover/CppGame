# 雷霆战机 (Thunder Fighter) — 完整设计方案

## 1. 整体架构：6 智能体 × 6 领地

```
项目根目录/
│
├── src/
│   ├── common/          ← ① Common Agent
│   ├── logic/           ← ② Logic Agent (Model + ViewModel 合并)
│   ├── view/            ← ③ View Agent
│   ├── resource/        ← ④ Resource Agent
│   └── app/             ← ⑤ App Agent
│
├── tests/               ← ⑥ Test Agent
│
├── include/             ← 各 Agent 的头文件（与 src 对应子目录）
├── resources/           ← 图片等静态素材（由 Resource Agent 加载）
├── CMakeLists.txt
└── DESIGN_PLAN.md
```

---

## 2. 六个智能体详解

---

### ① Common Agent（公共基础设施层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/common/` 目录下的所有文件 |
| **职责** | 系统最底层。存放所有其他智能体都会用到的基础工具 |
| **具体内容** | • 常量定义（屏幕宽高、帧率）<br>• 数学工具类（坐标计算、二维向量、碰撞检测用的圆/矩形）<br>• 日志打印（Logger）<br>• 公共枚举和类型定义（EntityType, Direction, GameState 等） |
| **越界限制** | **严禁** `#include` 系统里任何其他智能体的文件。它完全独立。 |

```
src/common/
├── Types.hpp            ← 枚举：EntityType, GameState, Direction, GameMode
├── Constants.hpp        ← 常量：SCREEN_WIDTH, SCREEN_HEIGHT, FPS
├── MathUtils.hpp/cpp    ← 向量运算、距离计算
├── Logger.hpp/cpp       ← 日志打印工具
└── Geometry.hpp/cpp     ← 矩形/圆形碰撞检测基础形状
```

---

### ② Logic Agent（核心逻辑层：Model + ViewModel）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/logic/` 目录下的所有文件 |
| **职责** | • **Model**：维护飞机血量、计算碰撞、更新分数、管理敌人波次、BOSS 战、技能系统、道具系统<br>• **ViewModel**：暴露 Q_PROPERTY / Q_INVOKABLE 接口供 View 调用<br>• 二者归同一个 Agent，因为 Model 加属性时 ViewModel 必须同步修改 |
| **越界限制** | • 可以访问 Common Agent<br>• **绝对不能**访问 View 层（严禁引入任何 Qt 头文件中的 UI 类）<br>• 它连屏幕长什么样都不知道，只管理数据和规则 |

```
src/logic/
├── GameModel.hpp/cpp         ← 游戏主模型：聚合所有子模块
├── Player.hpp/cpp            ← 玩家：坐标、生命、武器等级、无敌计时、技能CD
├── AircraftStats.hpp/cpp     ← 5架战机的属性模板（速度/火力/技能类型）
├── SkillSystem.hpp/cpp       ← 主动技能系统：释放、冷却、效果（全屏炸弹/护盾/加速等）
├── Enemy.hpp/cpp             ← 敌机：普通怪 + Boss 的 AI 行为模式
├── Bullet.hpp/cpp            ← 子弹：归属、坐标、速度、伤害
├── CollisionSystem.hpp/cpp   ← 碰撞检测：子弹 vs 敌机、敌机 vs 玩家
├── WaveManager.hpp/cpp       ← 闯关模式(7关) + 无尽模式的波次/难度表
├── PowerUpManager.hpp/cpp    ← 道具系统：回血、火力加强、护盾掉落及效果
├── ScoreManager.hpp/cpp      ← 计分、最高分、无尽模式资源（升级用）
└── GameViewModel.hpp/cpp     ← ViewModel：Q_PROPERTY + Q_INVOKABLE + signal
```

**关键约束**：`Logic/GameViewModel.hpp` 中 `#include <QObject>` 是唯一允许的 Qt 依赖，且 **不能** `#include` 任何 `QtQuick` 或 `QML` 相关的头文件。

---

### ③ View Agent（视图表现层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/view/` 目录下的所有文件 |
| **职责** | 专门和 Qt 库打交道，负责渲染画面、捕获用户按键。 |
| **具体内容** | • QML 主窗口 + 战机选择界面<br>• 模式选择界面（闯关/无尽）<br>• 游戏画布渲染<br>• HUD 显示（分数、生命、技能CD、关卡）<br>• BOSS 战血条<br>• 键盘按键捕获→通过命令绑定转发给 ViewModel<br>• 爆炸粒子特效、道具拾取反馈 |
| **严格 MVVM 约束** | View **不能直接读取/调用** ViewModel。所有交互必须通过**三绑定**：<br>① **属性绑定** — QML 声明式绑定 ViewModel 的 Q_PROPERTY（`text: vm.score`）<br>② **命令绑定** — Q_INVOKABLE 方法响应操作（`onClicked: vm.startGame()`）<br>③ **事件绑定** — ViewModel 的 signal 驱动 View 更新 |
| **越界限制** | • 可以访问 Common Agent（工具/常量）<br>• **不能**直接读取 Logic Agent 的 ViewModel——只能用 QML 声明式绑定语法<br>• **不能**直接修改 Model 里的底层数据 |
| **跨框架说明** | 当前用 Qt 的 Q_PROPERTY/Q_INVOKABLE/signal 实现三绑定。如果以后换框架，View 层整体重写，绑定语法换成新框架的（WPF 用 `{Binding}`，Web 用 React state 等），但三绑定的**设计模式不变**。 |

```
src/view/
├── main.qml                   ← 主窗口：ApplicationWindow
├── AircraftSelectScreen.qml   ← 战机选择界面（5架战机展示+属性对比）
├── ModeSelectScreen.qml       ← 模式选择（闯关模式 / 无尽模式）
├── GameCanvas.qml             ← 游戏画布：渲染所有实体
├── HUD.qml                    ← 抬头显示：分数、生命、技能CD、关卡/波次
├── BossHealthBar.qml          ← BOSS 血条（特殊显示）
├── PauseOverlay.qml           ← 暂停覆盖层
├── StageClearScreen.qml       ← 关卡通关界面（评分+进入下一关）
├── GameOverScreen.qml         ← 游戏结束界面（分数+资源统计）
├── UpgradeScreen.qml          ← 无尽模式：战机升级界面
├── PlayerPlane.qml            ← 玩家飞机图形组件（根据选择的战机显示不同图片）
├── EnemyPlane.qml             ← 敌机图形组件
├── BulletShape.qml            ← 子弹图形组件
├── StarField.qml              ← 星空粒子背景
└── ExplosionEffect.qml        ← 爆炸特效（普通爆炸 + BOSS爆炸）
```

---

### ④ Resource Agent（资源与持久化管理层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/resource/` 目录下的所有文件 |
| **职责** | 负责从硬盘读取/写入文件。 |
| **具体内容** | • 加载 PNG 图片（5架战机、多种敌机、BOSS、子弹、道具等）<br>• 读取/写入最高分记录<br>• 读取/写入无尽模式升级存档（每架战机的升级数据）<br>• 缓存已加载的资源，避免重复读盘 |
| **为什么需要它** | 把磁盘 I/O（读写文件）单独剥离出来。View 层需要图片时，只能向 Resource Agent **申请**，不能自己去读硬盘。 |

```
src/resource/
├── AssetManager.hpp/cpp       ← getImage("playerShip_1") → QPixmap
└── SaveManager.hpp/cpp        ← 读写最高分 + 无尽模式升级存档（JSON）
```

---

### ⑤ App Agent（应用生命周期层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/src/app/` 目录下的文件（main.cpp, Application.cpp） |
| **职责** | 系统的"总指挥"。包含 main 函数，负责在游戏启动时，把 View、Logic、Resource 实例化出来，并把它们"组装"在一起（依赖注入）。 |
| **越界限制** | • 它站在最顶层，认识所有人<br>• **里面绝对不写**任何具体的游戏逻辑或画图代码 |

```
src/app/
├── AppAgent.hpp/cpp           ← 应用初始化、依赖注入
└── main.cpp                   ← 程序入口
```

---

### ⑥ Test Agent（测试层）

| 项目 | 内容 |
|---|---|
| **负责文件** | `/tests/` 目录下的所有文件 |
| **职责** | 保证代码质量。不启动 Qt 窗口，而是在后台直接调用 Logic Agent 的代码进行单元测试。 |
| **越界限制** | 只能访问 Common 和 Logic，专门做单元测试。 |

```
tests/
├── test_main.cpp              ← Catch2 测试入口
├── test_game_model.cpp        ← Model 逻辑测试
├── test_view_model.cpp        ← ViewModel 桥接测试
├── test_collision.cpp         ← 碰撞检测专项测试
├── test_skill_system.cpp      ← 技能系统测试
└── test_wave_manager.cpp      ← 波次管理测试
```

---

## 3. 层间依赖关系

```
┌────────────────────────────────────────────────────────────┐
│  ⑤ App Agent     认识所有人，只组装，不写逻辑                │
├────────────────────────────────────────────────────────────┤
│  ③ View Agent    不认识 Logic（只通过三绑定与 VM 交互）    │
├────────────────────────────────────────────────────────────┤
│  ② Logic Agent   认识 Common，绝对不认识 View              │
├────────────────────────────────────────────────────────────┤
│  ④ Resource Agent 认识 Common，不认识 View/Logic           │
├────────────────────────────────────────────────────────────┤
│  ① Common Agent  不认识任何人，被所有人认识                  │
├────────────────────────────────────────────────────────────┤
│  ⑥ Test Agent    认识 Common + Logic，做单元测试           │
└────────────────────────────────────────────────────────────┘
```

**依赖方向**：`Common ← Logic ← View ← App`，上层可调用下层，**下层绝不调用上层**。

**View ↔ ViewModel 通信（三绑定，不是代码依赖）：**

```
ViewModel (C++ QObject)                       View (QML)
─────────────────────────                     ───────────
Q_PROPERTY(int score)        ──属性绑定──→    Text { text: vm.score }
Q_INVOKABLE void start()    ←──命令绑定───    Button { onClicked: vm.start() }
signal gameOver()            ──事件绑定──→    onGameOver: showOverlay()
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
    │ 第1关 → 第2关 →   │       │ 小怪波次 → BOSS   │
    │ ... → 第7关 BOSS  │       │ → 继续 → 更强BOSS │
    │ 通关 → 结算评分   │       │ → ... (无限循环)  │
    └──────────────────┘       └──────────────────┘
              │                             │
              │                      ┌──────┴──────┐
              │                      │ 升级界面      │
              │                      │ (用资源强化   │
              │                      │  战机属性)    │
              │                      └──────┬──────┘
              │                             │
              └──────────┬──────────────────┘
                         ▼
                   ┌──────────┐
                   │ 返回选择  │
                   └──────────┘
```

### 4.2 五架战机

| 编号 | 名称风格 | 基础火力 | 基础生命 | 移动速度 | 主动技能 | 技能效果 |
|---|---|---|---|---|---|---|
| ① | **雷霆号** | ★★★ | ★★★ | ★★★ | 雷暴领域 | 全屏雷击，对敌机造成大量伤害 |
| ② | **烈焰号** | ★★★★★ | ★★ | ★★ | 火焰风暴 | 前方扇形范围持续火焰喷射 |
| ③ | **冰霜号** | ★★ | ★★★★★ | ★★ | 极寒护盾 | 生成护盾吸收伤害 + 冻结附近敌机 |
| ④ | **幻影号** | ★★★ | ★★ | ★★★★★ | 时空闪避 | 瞬间向前冲刺，路径上所有敌机受到伤害 |
| ⑤ | **堡垒号** | ★★ | ★★★★ | ★ | 铁壁阵 | 短暂无敌 + 反弹敌方子弹 |

### 4.3 闯关模式（7 关）

| 关卡 | 主题 | 敌机配置 | BOSS | 难度 |
|---|---|---|---|---|
| 第1关 | 初入战场 | 小型机为主 | 轻型BOSS | ★☆☆ |
| 第2关 | 空中走廊 | 小型+中型 | 中型BOSS | ★★☆ |
| 第3关 | 雷云风暴 | 中型为主，偶尔大型 | 雷电BOSS | ★★☆ |
| 第4关 | 敌军要塞 | 中型+大型混编 | 重型BOSS | ★★★ |
| 第5关 | 暗夜突袭 | 大量小型机 + 精英 | 隐形BOSS | ★★★ |
| 第6关 | 火力封锁 | 全类型高密度 | 装甲BOSS | ★★★★ |
| 第7关 | 最终决战 | 最强配置 | 终极BOSS | ★★★★★ |

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

| 属性 | 初始值 | 无尽模式可升级 |
|---|---|---|
| 生命 | 3 | ✅ 每级 +1 上限 |
| 移动速度 | 归一化 0.6/秒 | ✅ 每级 +0.05 |
| 武器等级 | 1（战斗中最高 5） | ✅ 基础火力可永久升级 |
| 射击间隔 | 0.2秒（随武器等级递减） | ✅ 冷却缩减可升级 |
| 技能冷却 | 各战机不同，15~25秒 | ✅ 冷却缩减可升级 |

### 4.8 敌机类型

| 类型 | 生命 | 速度 | 分数 | 说明 |
|---|---|---|---|---|
| 小型机 | 1 | 0.2~0.4 | 10 | 直线下飞 |
| 中型机 | 2 | 0.15~0.3 | 30 | 左右摆动 |
| 大型机 | 5 | 0.1~0.2 | 50 | 会发射子弹 |
| BOSS | 50~200 | 0.05~0.1 | 500+ | 多种攻击模式 + 阶段转换 |

---

## 5. 数据流示例：完整游戏循环

```
每一帧 (60fps):

  App Agent 的 QML Timer 触发
       │
       ▼
  View Agent:     gameCanvas.tick(0.016)
       │ 调用 ViewModel
       ▼
  Logic Agent:    GameViewModel.tick(dt)
       │              ├─ GameModel.update(dt)
       │              │    ├─ Player.update(dt)           ← 无敌计时、技能CD
       │              │    ├─ Bullet.update(dt)           ← 子弹飞行
       │              │    ├─ Enemy.update(dt)            ← 敌机 AI 移动
       │              │    ├─ WaveManager.update(dt)      ← 判断生成/波次/BOSS
       │              │    ├─ PowerUpManager.update(dt)   ← 道具生成/拾取
       │              │    ├─ SkillSystem.update(dt)      ← 技能冷却、效果持续
       │              │    ├─ CollisionSystem.check()     ← 碰撞检测
       │              │    └─ ScoreManager.update()       ← 计分
       │              │
       │              └─ emit entitiesChanged()
       │              └─ emit scoreChanged()
       │              └─ emit livesChanged()
       │              └─ emit skillCooldownChanged()
       │              └─ emit bossHealthChanged()
       ▼
  View Agent:     收到信号 → 重绘画面
       │              ├─ 玩家飞机新位置
       │              ├─ 所有敌机/子弹新位置
       │              ├─ HUD 更新分数/生命/技能CD
       │              ├─ BOSS 血条更新
       │              └─ 爆炸特效播放
       │
       ▼
  Resource Agent:  需要新图片时 → AssetManager.getImage(name)
```

---

## 6. 文件修改权限矩阵

| 文件路径 | 归属 Agent | 谁可以修改 |
|---|---|---|
| `src/common/Types.hpp` | ① Common | **仅** ① |
| `src/logic/GameModel.cpp` | ② Logic | **仅** ② |
| `src/logic/Player.cpp` | ② Logic | **仅** ② |
| `src/logic/SkillSystem.cpp` | ② Logic | **仅** ② |
| `src/logic/WaveManager.cpp` | ② Logic | **仅** ② |
| `src/logic/GameViewModel.cpp` | ② Logic | **仅** ② |
| `src/view/AircraftSelectScreen.qml` | ③ View | **仅** ③ |
| `src/view/GameCanvas.qml` | ③ View | **仅** ③ |
| `src/view/HUD.qml` | ③ View | **仅** ③ |
| `src/resource/SaveManager.cpp` | ④ Resource | **仅** ④ |
| `src/app/AppAgent.cpp` | ⑤ App | **仅** ⑤ |
| `tests/test_skill_system.cpp` | ⑥ Test | **仅** ⑥ |
| `CMakeLists.txt` | 所有 Agent | 加新文件时对应 Agent 改 |

---

## 7. 图片资源映射表

### 7.1 玩家战机（你准备的）

| 文件 | 对应战机 | 说明 |
|---|---|---|
| `resources/images/myaircraft/Firepower.png` | ① 雷霆号 | 均衡型 |
| `resources/images/myaircraft/Flame.png` | ② 烈焰号 | 高火力 |
| `resources/images/myaircraft/Frost.png` | ③ 冰霜号 | 高血量 |
| `resources/images/myaircraft/Speed.png` | ④ 幻影号 | 极速 |
| `resources/images/myaircraft/Defense.png` | ⑤ 堡垒号 | 坦克 |

### 7.2 敌机（从 Kenney 包选取）

| 角色 | 文件 | 来源 | 说明 |
|---|---|---|---|
| **小型敌机** | `PNG/Enemies/enemyRed3.png` | Kenney | 红色倒三角，明显是杂兵 |
| **中型敌机** | `PNG/Enemies/enemyBlack1.png` | Kenney | 黑色装甲感，有威胁 |
| **大型敌机** | `PNG/Enemies/enemyBlack5.png` | Kenney | 最大号的黑色敌机 |
| **BOSS（第7关）** | `PNG/Sprites/Ships/spaceShips_009.png` | Kenney | 大型飞船，适合做最终BOSS |
| **BOSS（第4~6关）** | `PNG/Sprites/Rockets/spaceRockets_001.png` | Kenney | **大火箭！** |
| **BOSS（第2~3关）** | `PNG/Sprites/Ships/spaceShips_004.png` | Kenney | 中型飞船 |

> BOSS 总共 7 关，上面 4 种交替使用+变色/放大即可，不需要 7 张不同的图。
> 例如：Rockets 用于前半关卡，Ships 用于后半关卡。

### 7.3 子弹（从 Kenney 包选取）

| 角色 | 文件 | 说明 |
|---|---|---|
| **玩家子弹** | `PNG/Lasers/laserBlue16.png` | 蓝色长条激光弹 |
| **敌方普通子弹** | `PNG/Lasers/laserRed01.png` | 红色圆形弹 |
| **敌方导弹** | `PNG/Sprites/Missiles/spaceMissiles_001.png` | 可选，给大型敌机/BOSS用 |

### 7.4 特效（从 Kenney 包选取）

| 效果 | 文件 | 说明 |
|---|---|---|
| **爆炸动画** | `PNG/Effects/fire00.png` ~ `fire19.png` | 20 帧逐帧爆炸 |
| **引擎火焰** | `PNG/Sprites/Effects/engine1.png` ~ `engine5.png` | 战机尾部推进火焰 |
| **护盾** | `PNG/Effects/shield1~3.png` | 冰霜号/堡垒号技能用 |
| **加速图标** | `PNG/Effects/speed.png` | 幻影号技能用 |
| **星星粒子** | `PNG/Effects/star1~3.png` | 背景/道具用 |

### 7.5 道具（从 Kenney 包选取）

| 道具 | 文件 | 说明 |
|---|---|---|
| ❤️ **回血包** | `PNG/Power-ups/pill_red.png` | 红色药丸 |
| ⚡ **火力加强** | `PNG/Power-ups/powerupBlue_bolt.png` | 蓝色闪电 |
| 🛡️ **护盾** | `PNG/Power-ups/shield_gold.png` | 金色护盾 |
| ⭐ **星核碎片** | `PNG/Power-ups/star_gold.png` | 金色星星 |

### 7.6 背景

| 用途 | 文件 | 说明 |
|---|---|---|
| **游戏背景** | `PNG/Backgrounds/darkPurple.png` | 深紫星空 |

### 7.7 攻击方式说明（不依赖额外图片）

所有攻击方式的差异由 **代码控制**，不是靠不同图片：

| 战机 | 子弹图 | 发射模式 | 射速 |
|---|---|---|---|
| 雷霆号 | laserBlue16.png | 双发平行 | 中等 |
| 烈焰号 | laserBlue16.png | 3 发散射 | 稍慢 |
| 冰霜号 | laserBlue16.png | 单发 + 减速效果 | 中等 |
| 幻影号 | laserBlue16.png | 快速单发 | 极快 |
| 堡垒号 | laserBlue16.png | V 形双发 | 中等 |

技能效果同样由 QML 动画 + Kenney 特效图实现，不需要额外图片。

### 7.8 BOSS 攻击方式（纯代码控制）

BOSS 的攻击通过**代码配置攻击模式**实现，不依赖图片：

```
BOSS 攻击模式示例（第1关 BOSS）:
  阶段1: 左右平移 + 每2秒发射1发红色子弹
  阶段2: 血量<50% → 加速 + 每1.5秒发射3发散弹
```

所有 BOSS 共用 `laserRed01.png` 作为子弹图，区别只在于数量、角度、频率。

---

## 8. 迭代重构规划

> **核心思路**：每次迭代产出 **一个可运行的游戏版本**，然后在此基础上重构/扩展，逐步逼近最终设计。
> 每次迭代完成后 → 代码上传 GitHub。

---

### 迭代 1 — MVP 最小可玩版 🎯

**目标**：能"飞起来、打出去、撞上去、死掉重来"的最简闭环。

| 维度 | 内容 |
|---|---|
| **战机** | 固定 1 架（先用 Kenney 的 playerShip1_blue），无选择 |
| **模式** | 单一无尽波次，无闯关 |
| **敌机** | 只有小型机，从上方直线下飞 |
| **BOSS** | 无 |
| **技能** | 无 |
| **道具** | 无 |
| **波次** | 简单的定时生成，无限波次 |
| **UI** | 开始画面 + 游戏画面 + HUD(分数/生命) + 结束画面 |
| **操作** | WASD 移动 + 自动开火 |
| **存档** | 最高分读写 |
| **可用图片** | 先用 Kenney 素材 |

**涉及 Agent**：Common → Logic(Player/Enemy/Bullet/Collision/Score) → View(GameCanvas/HUD) → Resource → App

**可交付判断**：打开游戏 → 开始 → 控制飞机移动射击 → 敌机被击落 → 撞到敌机扣血 → 血量为零 → Game Over

---

### 迭代 2 — BOSS 战 + 7 关闯关 🏆

**目标**：有 BOSS、有通关流程、有道具掉落。

| 维度 | 新增/变化 |
|---|---|
| **敌机** | 新增中型机、大型机 |
| **BOSS** | 实现 BOSS AI（阶段转换、多种攻击模式） |
| **模式** | 新增闯关模式，设计 7 关波次表 |
| **道具** | 回血包 + 火力加强 + 护盾（战斗中掉落） |
| **UI** | 新增模式选择界面、关卡选择/过渡画面、BOSS 血条 |
| **Logic 新增** | `WaveManager` 重构支持关卡表、`PowerUpManager`、BOSS AI |
| **存档** | 闯关进度保存 |

**可交付判断**：闯关模式可选 → 7 关可打 → 每关有 BOSS → 道具掉落拾取有效

---

### 迭代 3 — 5 战机 + 技能系统 🚀

**目标**：战机可选，各有特色技能。

| 维度 | 新增/变化 |
|---|---|
| **战机** | 实现 5 架战机属性模板（雷霆/烈焰/冰霜/幻影/堡垒） |
| **技能** | 实现 SkillSystem：5 种主动技能 + CD 管理 + 视觉效果 |
| **UI** | 新增战机选择界面（展示 5 架战机的属性对比） |
| **UI** | HUD 新增技能 CD 显示、战机名称显示 |
| **操作** | Space 键释放技能 |
| **View 新增** | `AircraftSelectScreen.qml` |
| **Logic 新增** | `AircraftStats.hpp`, `SkillSystem.hpp` |
| **图片** | 需要 5 张不同的战机图片 |

**可交付判断**：开始可选战机 → 不同战机属性/外观不同 → 按 Space 放技能有效 → CD 正常

---

### 迭代 4 — 无尽模式 + 升级系统 🔄

**目标**：无尽模式可玩，有成长感。

| 维度 | 新增/变化 |
|---|---|
| **模式** | 无尽模式完整实现 |
| **资源** | 星核碎片掉落系统 |
| **升级** | 战机永久升级（火力/生命/速度/冷却） |
| **UI** | 新增升级界面、资源数量显示 |
| **存档** | 升级数据持久化 |
| **难度曲线** | 无尽模式每轮递增 10% 难度 |
| **View 新增** | `UpgradeScreen.qml` |
| **Logic 新增** | 升级系统逻辑 |

**可交付判断**：无尽模式可选 → 击杀敌机掉落星核 → 可进入升级界面加点 → 升级效果在战斗中体现 → 下次启动游戏升级保留

---

### 迭代 5 — 画面精修 + 平衡调整 ✨

**目标**：换掉不满意的图片，调数值，修 Bug。

| 维度 | 内容 |
|---|---|
| **图片替换** | 你找到更好的主战机/BOSS 图后，我来适配 |
| **特效增强** | 爆炸粒子、引擎火焰、技能特效美化 |
| **数值平衡** | 敌机血量/速度、BOSS 难度、升级数值调整 |
| **Bug 修复** | 碰撞检测边界、UI 闪烁、内存泄漏 |
| **性能优化** | 实体池、减少 QML 对象创建 |
| **GitHub** | 最终版提交 + README 文档 |

---

## 9. 游戏设计核心循环总结

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
