# 雷霆战机 — 多智能体开发规则书 (RULES)

## 目录

1. [开发环境](#1-开发环境)
2. [六个智能体总览](#2-六个智能体总览)
3. [各智能体详细规则](#3-各智能体详细规则)
4. [CMake 构建规则（编译隔离 + 统一链接）](#4-cmake-构建规则)
5. [Git 提交规则](#5-git-提交规则)
6. [常见违规检查清单](#6-常见违规检查清单)

---

## 1. 开发环境

### 1.1 技术栈

| 项目 | 版本/值 |
|---|---|
| 语言标准 | C++20 |
| UI 框架 | Qt5 (Core / Gui / Quick / Qml) |
| 构建系统 | CMake ≥ 3.20 |
| 包管理器 | vcpkg (triplet: x64-mingw-dynamic) |
| 测试框架 | Catch2 |
| 编译器 | MinGW (GCC) |
| 图形资源 | Kenney Space Shooter Redux (PNG) |

### 1.2 可用 Qt 模块

| 模块 | 允许使用的 Agent |
|---|---|
| `Qt5::Core` | Logic, View, App, Resource, Test |
| `Qt5::Gui` | View のみ（渲染需要） |
| `Qt5::Quick` | View のみ（QML 引擎） |
| `Qt5::Qml` | View + App（QML 注册） |
| `Qt5::Gamepad` | 本项目不使用 |

### 1.3 目录结构

```
thunder-fighter/
├── src/
│   ├── common/          ← ① Common Agent 领地
│   ├── logic/           ← ② Logic Agent 领地
│   ├── view/            ← ③ View Agent 领地
│   ├── resource/        ← ④ Resource Agent 领地
│   └── app/             ← ⑤ App Agent 领地
├── tests/               ← ⑥ Test Agent 领地
├── resources/           ← 素材存放（由 Resource Agent 读取）
├── include/             ← 镜像 src/ 结构放头文件
├── CMakeLists.txt
└── vcpkg.json
```

---

## 2. 六个智能体总览

| # | 智能体 | 目录 | 一句话职责 | 依赖 |
|---|---|---|---|---|
| ① | **Common** | `src/common/` | 提供类型/工具/常量，被所有人用 | ❌ 不依赖任何人 |
| ② | **Logic** | `src/logic/` | 游戏规则 + ViewModel 桥接 | 依赖 ① |
| ③ | **View** | `src/view/` | 渲染画面 + 收集按键 | 依赖 ① ② |
| ④ | **Resource** | `src/resource/` | 图片加载 + 存档读写 | 依赖 ① |
| ⑤ | **App** | `src/app/` | 启动组装 + 依赖注入 | 依赖 ① ② ③ ④ |
| ⑥ | **Test** | `tests/` | 单元测试 + 集成测试 | 依赖 ① ② |

---

## 3. 各智能体详细规则

---

### ① Common Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/common/` + `include/common/` |
| **负责内容** | 枚举定义、常量、数学工具(向量/距离)、日志打印、基础几何形状 |
| **可读（可 include）** | ❌ 无。不得 include 任何本项目其他文件 |
| **可写（可修改）** | 仅限自己目录下的文件 |
| **不可读/不可碰** | 任何其他 Agent 的文件（`src/logic/`, `src/view/`, `src/resource/`, `src/app/`, `tests/`） |
| **接口（提供给外界的）** | `Types.hpp` 的枚举、`MathUtils` 的向量函数、`Constants` 的常量 |

**示例产出文件：**
```
src/common/
├── Types.hpp          → enum class EntityType { Player, EnemySmall, ... };
├── Constants.hpp      → constexpr int SCREEN_WIDTH = 800;
├── MathUtils.hpp/cpp  → struct Vec2 { float x,y; }; float distance(Vec2, Vec2);
├── Logger.hpp/cpp     → void log(const std::string& msg);
└── Geometry.hpp/cpp   → struct Rect { float x,y,w,h; }; bool overlaps(Rect, Rect);
```

---

### ② Logic Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/logic/` + `include/logic/` |
| **负责内容** | **Model**：玩家/敌机/子弹状态、碰撞检测、计分、波次<br>**ViewModel**：Q_PROPERTY + Q_INVOKABLE + signal 暴露给 View |
| **可读** | ✅ 可读 Common Agent 的所有文件<br>❌ **不可读** View 的任何文件<br>❌ **不可读** Resource 的实现细节（但可通过接口调用） |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | `src/view/` — **绝对不能** include 任何 QtQuick 或 QML 文件<br>`tests/` — 不写测试代码 |
| **Qt 限制** | 只能 `#include <QObject>` 和 `#include <QVariant>`，**不能** `#include <QtQuick>` 或 `#include <QGuiApplication>` |

**详细文件清单：**
```
src/logic/
├── GameModel.hpp/cpp         ← 聚合所有子模块，提供顶层 update()/reset()
├── Player.hpp/cpp            ← 玩家位置、生命、武器等级、无敌计时
├── Enemy.hpp/cpp             ← 敌机类型、坐标、AI 行为（移动模式）
├── Bullet.hpp/cpp            ← 子弹归属(玩家/敌方)、坐标、速度、伤害
├── CollisionSystem.hpp/cpp   ← 碰撞检测：子弹 vs 敌机、敌机 vs 玩家
├── WaveManager.hpp/cpp       ← 波次管理：生成节奏、难度递增表
├── ScoreManager.hpp/cpp      ← 计分规则、最高分（调用 Resource 读写）
└── GameViewModel.hpp/cpp     ← Q_PROPERTY Q_INVOKABLE signal
```

---

### ③ View Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/view/`（含 QML 资源） |
| **负责内容** | 画面渲染、按键捕获、动画特效、UI 布局 |
| **可读** | ✅ 可读 Common Agent<br>❌ **不可读** Logic Agent（严格 MVVM：View 不直接读取 ViewModel）<br>❌ **不可读** Resource 的实现 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | `src/logic/`、`src/resource/`、`src/app/` |
| **关键约束** | **严格 MVVM 三绑定原则**：<br>① **属性绑定** — QML 声明式绑定 ViewModel 的 Q_PROPERTY（如 `text: vm.score`）<br>② **命令绑定** — 用户操作通过 Q_INVOKABLE 转发给 ViewModel（如 `onClicked: vm.startGame()`）<br>③ **事件绑定** — ViewModel 的 signal 驱动 View 更新（如 `onScoreChanged`）<br>View **绝不能**在 C++ 代码里 include ViewModel 的头文件去调 getter |

**三绑定图解（严格 MVVM）：**
```
ViewModel 层 (C++ QObject)          View 层 (QML)
──────────────────────────────      ──────────────────────
Q_PROPERTY(int score ...)    ──①──→ Text { text: vm.score }
Q_INVOKABLE void start()    ←─②─── Button { onClicked: vm.start() }
signal gameOver()           ──③──→ onGameOver: { ... }
```

**"读取" vs "绑定" 的区别：**

| ❌ 主动读取（违规） | ✅ 声明式绑定（合规） |
|---|---|
| `int s = viewModel->getScore();` | `text: vm.score`（框架自动取值） |
| `viewModel->setScore(10);` | 不写这行，由 ViewModel 自己改 |
| C++ 代码 include ViewModel.hpp | QML 只通过 contextProperty 接触 VM |

**详细文件清单：**
```
src/view/
├── main.qml                   ← 主窗口 ApplicationWindow
├── GameCanvas.qml             ← 游戏画布（用 Image + Repeater 渲染实体）
├── HUD.qml                    ← 分数/生命/武器等级显示
├── StartScreen.qml            ← 开始界面
├── GameOverScreen.qml         ← 游戏结束界面
├── PauseOverlay.qml           ← 暂停覆盖层
├── PlayerPlane.qml            ← 玩家飞机（Image + 引擎火焰动画）
├── EnemyPlane.qml             ← 敌机（Image + 生命条）
├── BulletShape.qml            ← 子弹（Image 发光效果）
├── StarField.qml              ← 星空粒子背景
└── ExplosionEffect.qml        ← 爆炸特效
```

---

### ④ Resource Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/resource/` + `include/resource/` |
| **负责内容** | 加载 PNG 图片到内存（QPixmap/QImage）、读写最高分 JSON 文件 |
| **可读** | ✅ 可读 Common Agent<br>❌ **不可读** Logic 的任何文件<br>❌ **不可读** View 的任何文件 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | `src/logic/` 的游戏规则、`src/view/` 的渲染代码 |

**详细文件清单：**
```
src/resource/
├── AssetManager.hpp/cpp   ← getImage("playerShip1_blue") → QPixmap
└── SaveManager.hpp/cpp    ← loadHighScore() / saveHighScore(int)
```

**关键规则**：Logic 和 View **不能直接调用 QFile / QImage** 读写文件。所有文件 I/O 必须通过 Resource Agent。

---

### ⑤ App Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/app/` + `include/app/` |
| **负责内容** | main 函数、创建各 Agent 实例、依赖注入、启动 QML 引擎 |
| **可读** | ✅ 可读所有 Agent |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ⚠️ **绝对不能**在 App 代码中写任何游戏逻辑或渲染代码——它只做组装 |

**详细文件清单：**
```
src/app/
├── AppAgent.hpp/cpp   ← init() / run() / shutdown()
└── main.cpp           ← 入口：调用 AppAgent::init() → run()
```

---

### ⑥ Test Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `tests/` |
| **负责内容** | 单元测试 Common 和 Logic 的代码正确性 |
| **可读** | ✅ 可读 Common Agent<br>✅ 可读 Logic Agent（被测对象）<br>❌ **不可读** View 层（测试不启动 Qt 窗口）<br>❌ **不可读** Resource 层（测试用 Mock 代替文件 I/O） |
| **可写** | 仅限自己目录下的文件 |

**详细文件清单：**
```
tests/
├── test_main.cpp              ← Catch2 主入口
├── test_game_model.cpp        ← 测试 GameModel 的碰撞/计分逻辑
└── test_view_model.cpp        ← 测试 ViewModel 的信号转发
```

---

## 4. CMake 构建规则

### 4.1 为什么要隔离 + 还能统一链接？

每个 Agent 编译成一个 **OBJECT 库**，然后最终可执行文件把它们全部链接进来：

```
逻辑：
  Agent A 编译时只能看到自己的头文件 + 它依赖的 Agent 的头文件
  → 编译阶段就防止了越界 include

  最后所有 OBJECT 库合成一个 exe
  → 运行时是一个整体，正常游戏
```

### 4.2 CMake 结构示例

```cmake
# ▸ Common Agent — 不依赖任何人
add_library(common OBJECT
    src/common/MathUtils.cpp
    src/common/Logger.cpp
    src/common/Geometry.cpp
)
target_include_directories(common PUBLIC include/common)
# 没有 target_link_libraries(common ...) ← Common 不依赖任何人

# ▸ Logic Agent — 依赖 Common
add_library(logic OBJECT
    src/logic/GameModel.cpp
    src/logic/Player.cpp
    src/logic/Enemy.cpp
    src/logic/Bullet.cpp
    src/logic/CollisionSystem.cpp
    src/logic/WaveManager.cpp
    src/logic/ScoreManager.cpp
    src/logic/GameViewModel.cpp
)
target_include_directories(logic PUBLIC include/logic)
target_link_libraries(logic PUBLIC common)   # ← Logic 可以看到 Common

# ▸ View Agent — 依赖 Common + Logic
add_library(view OBJECT
    # QML 文件通过 qrc 注册
)
target_link_libraries(view PUBLIC common logic Qt5::Quick Qt5::Qml)

# ▸ Resource Agent — 依赖 Common
add_library(resource OBJECT
    src/resource/AssetManager.cpp
    src/resource/SaveManager.cpp
)
target_link_libraries(resource PUBLIC common)

# ▸ App Agent — 依赖所有
add_executable(ThunderFighter
    src/app/main.cpp
    src/app/AppAgent.cpp
    $<TARGET_OBJECTS:common>
    $<TARGET_OBJECTS:logic>
    $<TARGET_OBJECTS:view>
    $<TARGET_OBJECTS:resource>
)

# ▸ Test — 依赖 Common + Logic
add_executable(ThunderFighter_tests
    tests/test_main.cpp
    tests/test_game_model.cpp
    tests/test_view_model.cpp
    $<TARGET_OBJECTS:common>
    $<TARGET_OBJECTS:logic>
)
target_link_libraries(ThunderFighter_tests PRIVATE Catch2::Catch2WithMain)
```

**这样：**
- 编译时 Logic 想 `#include` View 的头文件 → **编译报错** ❌
- 运行时所有代码在一个 exe 里 → **正常运行** ✅
- Test 可以单独编译运行，不依赖 Qt 窗口 → **CI 友好** ✅

---

## 5. Git 提交规则

### 5.1 基本原则

> **只在大范围重构后提交，小修改不提交。**

### 5.2 提交时机

| 场景 | 是否提交 |
|---|---|
| 写完 Phase 1（项目骨架搭建完成） | ✅ 提交 |
| 写完 Logic Agent 的全部核心逻辑 | ✅ 提交 |
| 写完 View Agent + 能跑起来了 | ✅ 提交 |
| 修了一个变量名拼写错误 | ❌ 不提交 |
| 调了一行颜色值 | ❌ 不提交 |
| 加了一个测试用例 | ❌ 不提交 |
| 整个游戏可玩 Beta 版 | ✅ 提交 |

### 5.3 提交信息格式

```
<Agent-Name>: <概述>

<详细说明（可选）>
```

示例：
```
Logic: 实现碰撞检测 + 玩家生命系统

- Player 新增 lives / invincibleTimer / takeDamage()
- CollisionSystem 支持子弹-敌机、敌机-玩家碰撞
- GameModel 整合碰撞结果到状态更新流程
```

### 5.4 分支策略

- `main` — 稳定版本，只合入经过测试的大版本
- 开发过程中不需要开分支，直接在 main 上迭代

---

## 6. 常见违规检查清单

每个 Agent 在修改代码前，先自问：

| 问题 | Common | Logic | View | Resource | App | Test |
|---|---|---|---|---|---|---|
| 我 include 了不该 include 的 Agent 吗？ | — | 没 include View 吧？ | 没 include Model 的 .hpp 吧？ | 没 include Logic 吧？ | 没写业务逻辑吧？ | 没 include View 吧？ |
| 我改了不该我改的目录吗？ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 我直接读写文件了吗？ | — | 调用 Resource 了吗？ | 调用 Resource 了吗？ | — | — | 用 Mock 了吗？ |
| 我在 QML 里直接改 Model 了吗？ | — | — | 只通过 ViewModel 了吗？ | — | — | — |

---

> **这份规则书定义了 6 个智能体各自的领地、权限、约束。后续每个会话开工前，先读一遍对应的规则，确保不越界。** 🚀
