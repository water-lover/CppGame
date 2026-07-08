# 雷霆战机 — 多智能体开发规则书 (RULES)

## 目录

1. [设计依据](#1-设计依据)
2. [开发环境](#2-开发环境)
3. [六个智能体总览](#3-六个智能体总览)
4. [各智能体详细规则](#4-各智能体详细规则)
5. [三绑定详细规范](#5-三绑定详细规范)
6. [命令模式规范](#6-命令模式规范)
7. [CMake 构建规则](#7-cmake-构建规则)
8. [Git 提交规则](#8-git-提交规则)
9. [常见违规检查清单](#9-常见违规检查清单)

---

## 1. 设计依据

本架构严格遵循课件 MVVM 定义：

> **"ViewModel 层就是前述改进过的表现层。"**
> **"View 层和 ViewModel 层之间通过三个绑定来解开耦合：单向属性数据绑定、命令绑定、事件通知绑定。"**
> **"由于游戏类的数据即为可绘制对象，不需要转换，所以取消 Model 层，仅使用 ViewModel 层。"**
> **"可绘制对象有游戏地图和精灵图片两种，所以设计两个 ViewModel 类。"**
> **"使用 std::function 类实现命令模式。"**
> **"增加 viewmodel 子目录。"**

---

## 2. 开发环境

### 2.1 技术栈

| 项目 | 版本/值 |
|---|---|
| 语言标准 | C++20 |
| UI 框架 | Qt5 (Core / Gui / Widgets) |
| 渲染方案 | QGraphicsView + QGraphicsScene + QGraphicsItem（**纯 C++，无 QML**） |
| 构建系统 | CMake ≥ 3.20 |
| 包管理器 | vcpkg (triplet: x64-mingw-dynamic) |
| 测试框架 | Catch2 |
| 编译器 | MinGW (GCC) |
| 图形资源 | Kenney Space Shooter Redux (PNG) |

### 2.2 可用 Qt 模块

| 模块 | 允许使用的 Agent |
|---|---|
| `Qt5::Core` | ViewModel, View, App, Resource, Test |
| `Qt5::Gui` | View 仅限（QPainter 渲染需要） |
| `Qt5::Widgets` | View + App（主窗口 + QGraphicsView） |

> `Qt5::Quick` 和 `Qt5::Qml` **本项目不使用**。禁止任何 QML/JavaScript 文件或代码。

### 2.3 目录结构

```
thunder-fighter/
├── src/
│   ├── common/          ← ① Common Agent
│   ├── viewmodel/       ← ② ViewModel Agent（取消 Model 层）
│   ├── view/            ← ③ View Agent（纯 C++，无 QML）
│   ├── resource/        ← ④ Resource Agent
│   └── app/             ← ⑤ App Agent
├── tests/               ← ⑥ Test Agent
├── resources/           ← 素材存放（由 Resource Agent 读取）
├── include/
│   ├── common/          ← ①
│   ├── viewmodel/       ← ②
│   ├── view/            ← ③
│   ├── resource/        ← ④
│   └── app/             ← ⑤
├── CMakeLists.txt
└── vcpkg.json
```

---

## 3. 六个智能体总览

| # | 智能体 | 目录 | 一句话职责 | 依赖 |
|---|---|---|---|---|
| ① | **Common** | `src/common/` | 提供类型/工具/常量，被所有人用 | ❌ 不依赖任何人 |
| ② | **ViewModel** | `src/viewmodel/` | 游戏数据+规则，std::function 命令暴露给 View | 依赖 ① |
| ③ | **View** | `src/view/` | 纯 C++ QGraphicsView 渲染+按键捕获 | 依赖 ① ② |
| ④ | **Resource** | `src/resource/` | 图片加载+存档读写（唯一可读写磁盘的 Agent） | 依赖 ① |
| ⑤ | **App** | `src/app/` | 启动组装+建立三绑定连接 | 依赖 ① ② ③ ④ |
| ⑥ | **Test** | `tests/` | 单元测试 Common 和 ViewModel | 依赖 ① ② |

**关键设计原则：**

```
依赖方向：Common ← ViewModel ← View ← App
                          ↕ （通过三绑定，不是代码依赖）
```

- 上层可调用下层，**下层绝不能调用上层**
- ViewModel 不认识 View，只通过 Qt signal 和 std::function 暴露接口
- View 认识 ViewModel（include 其头文件），但只能通过三绑定交互
- App Agent 是唯一的组装者，负责 connect 信号和传递命令指针

---

## 4. 各智能体详细规则

---

### ① Common Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/common/` + `include/common/` |
| **负责内容** | 枚举定义、常量、数学工具(Vec2/距离)、日志打印、基础几何形状(Rect/Circle) |
| **可读（可 include）** | ❌ 无。不得 include 任何本项目其他文件 |
| **可写（可修改）** | 仅限自己目录下的文件 |
| **不可碰** | 任何其他 Agent 的文件（`src/viewmodel/`, `src/view/`, `src/resource/`, `src/app/`, `tests/`） |

**文件清单：**
```
src/common/                    include/common/
├── MathUtils.cpp              ├── MathUtils.hpp     ← Vec2, distance, normalize
├── Logger.cpp                 ├── Logger.hpp        ← log(tag, msg)
└── Geometry.cpp               ├── Geometry.hpp      ← Rect, Circle, overlaps
                                ├── Types.hpp         ← EntityType, GameState, Direction
                                └── Constants.hpp     ← SCREEN_WIDTH, PLAYER_SPEED, ...
```

---

### ② ViewModel Agent — 核心层（取消 Model 层）

| 规则项 | 内容 |
|---|---|
| **目录** | `src/viewmodel/` + `include/viewmodel/` |
| **负责内容** | 游戏全部数据和规则。**无独立 Model 层**（数据即可绘制对象，不需要类型转换）。两个 ViewModel 类。命令用 `std::function`。 |
| **可读** | ✅ 可读 Common Agent<br>❌ **不可读** View 的任何文件<br>❌ **不可读** Resource 的实现细节 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | `src/view/` — 不能 include 任何 UI 头文件<br>`src/resource/` — 不直接写文件（通过 SaveManager 接口调用） |
| **Qt 限制** | 只能 `#include <QObject>` 和 `#include <QVariant>`，**不能** include 任何 Widgets/Quick 头文件 |

**文件清单：**
```
src/viewmodel/                 include/viewmodel/
├── SpriteEntityVM.cpp         ├── SpriteEntityVM.hpp  ← 精灵实体 ViewModel（核心）
├── GameMapVM.cpp              ├── GameMapVM.hpp       ← 地图属性 ViewModel
├── Player.cpp                 ├── Player.hpp          ← 玩家数据类
├── Enemy.cpp                  ├── Enemy.hpp           ← 敌机数据类
├── Bullet.cpp                 ├── Bullet.hpp          ← 子弹数据类
├── CollisionSystem.cpp        ├── CollisionSystem.hpp ← 碰撞检测工具
├── ScoreManager.cpp           ├── ScoreManager.hpp    ← 计分+最高分
├── PowerUpManager.cpp (后续)  ├── PowerUpManager.hpp
├── AircraftStats.cpp (后续)   ├── AircraftStats.hpp
├── SkillSystem.cpp (后续)     ├── SkillSystem.hpp
└── WaveManager.cpp (后续)     └── WaveManager.hpp
```

---

### ③ View Agent — 纯 C++（无 QML）

| 规则项 | 内容 |
|---|---|
| **目录** | `src/view/` + `include/view/` |
| **技术方案** | Qt5 `QGraphicsView` + `QGraphicsScene` + 自定义 `QGraphicsItem` 子类 |
| **负责内容** | QPainter 渲染画面、捕获键盘输入、管理界面切换。**纯 C++，无 QML/JS** |
| **可读** | ✅ 可读 Common Agent<br>✅ **可读** ViewModel 的头文件（为了 connect 信号和持有 std::function 引用）<br>❌ **不可读** Resource 的实现细节 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ❌ **不能** include 数据类的头文件（Player.hpp / Enemy.hpp / Bullet.hpp）<br>❌ **不能**直接操作游戏数据<br>❌ **不能**直接读写文件 |
| **三绑定** | 参见第5节"三绑定详细规范" |

**文件清单：**
```
src/view/                      include/view/
├── GameView.cpp               ├── GameView.hpp        ← QGraphicsView 主窗口
├── PlayerItem.cpp             ├── PlayerItem.hpp      ← 玩家飞机
├── EnemyItem.cpp              ├── EnemyItem.hpp       ← 敌机
├── BulletItem.cpp             ├── BulletItem.hpp      ← 子弹
├── StarFieldItem.cpp          ├── StarFieldItem.hpp   ← 星空背景
├── HudOverlay.cpp             ├── HudOverlay.hpp      ← 分数/生命覆盖层
├── StartScreen.cpp            ├── StartScreen.hpp     ← 开始界面
├── GameOverScreen.cpp         ├── GameOverScreen.hpp  ← 游戏结束界面
├── AircraftSelectScreen.cpp   ├── AircraftSelectScreen.hpp  (后续)
├── ModeSelectScreen.cpp       ├── ModeSelectScreen.hpp      (后续)
├── BossHealthBar.cpp          ├── BossHealthBar.hpp         (后续)
├── PauseOverlay.cpp           ├── PauseOverlay.hpp          (后续)
├── StageClearScreen.cpp       ├── StageClearScreen.hpp      (后续)
├── UpgradeScreen.cpp          ├── UpgradeScreen.hpp         (后续)
└── ExplosionEffect.cpp        └── ExplosionEffect.hpp       (后续)
```

---

### ④ Resource Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/resource/` + `include/resource/` |
| **负责内容** | 加载 PNG 图片到 QPixmap（带缓存）、读写最高分/升级数据 JSON 文件 |
| **可读** | ✅ 可读 Common Agent<br>❌ **不可读** ViewModel 的任何文件<br>❌ **不可读** View 的任何文件 |
| **可写** | 仅限自己目录下的文件 |

```
src/resource/                  include/resource/
├── AssetManager.cpp           ├── AssetManager.hpp  ← getImage(key) → QPixmap
└── SaveManager.cpp            └── SaveManager.hpp   ← load/saveHighScore(int)
```

**关键规则**：ViewModel 和 View **绝对不能直接调用 QFile / QImage / QPixmap 读写文件**。所有文件 I/O 必须通过 Resource Agent。

---

### ⑤ App Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/app/` + `include/app/` |
| **负责内容** | main 函数、创建所有 Agent 实例、建立三绑定连接、启动 QTimer 帧循环 |
| **技术方案** | `QApplication` + `QGraphicsView`，**无** QQmlApplicationEngine |
| **可读** | ✅ 可读所有 Agent |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ⚠️ **绝对不能**写任何游戏逻辑或渲染代码——只做组装和连接 |

```
src/app/                       include/app/
├── AppAgent.cpp               ├── AppAgent.hpp  ← init() / run() / shutdown()
└── main.cpp                   └── (无头文件)
```

**AppAgent 的职责（示例流程）：**
```cpp
void AppAgent::init() {
    // 1. 创建所有 Agent
    auto* spriteVM = new SpriteEntityVM(this);
    auto* mapVM    = new GameMapVM(this);
    auto* gameView = new GameView(this);   // GameView 持有命令引用

    // 2. 属性绑定：ViewModel signal → View slot
    connect(spriteVM, &SpriteEntityVM::scoreChanged,     hud, &HudOverlay::setScore);
    connect(spriteVM, &SpriteEntityVM::livesChanged,     hud, &HudOverlay::setLives);
    connect(spriteVM, &SpriteEntityVM::playerPosChanged, playerItem, &PlayerItem::sync);
    connect(spriteVM, &SpriteEntityVM::enemiesChanged,   gameView, &GameView::syncEnemies);
    connect(spriteVM, &SpriteEntityVM::bulletsChanged,   gameView, &GameView::syncBullets);
    connect(mapVM,    &GameMapVM::scrollOffsetChanged,   starField, &StarFieldItem::setOffset);

    // 3. 事件绑定：ViewModel signal → View 界面切换
    connect(spriteVM, &SpriteEntityVM::gameStarted, gameView, &GameView::onGameStarted);
    connect(spriteVM, &SpriteEntityVM::gameOver,    gameView, &GameView::onGameOver);

    // 4. 命令绑定：View 已在构造函数中持有 std::function 引用

    // 5. 启动帧循环
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
        spriteVM->cmdTick(0.016f);
        mapVM->cmdUpdate(0.016f);
    });
    timer->start(16);
}
```

---

### ⑥ Test Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `tests/` |
| **负责内容** | 单元测试 Common 和 ViewModel 的代码正确性。不启动 Qt 窗口 |
| **可读** | ✅ 可读 Common Agent<br>✅ 可读 ViewModel Agent<br>❌ **不可读** View 层 |
| **可写** | 仅限自己目录下的文件 |

```
tests/
├── test_main.cpp              ← Catch2 入口（Catch2WithMain 已提供 main）
├── test_player.cpp            ← Player 移动/生命/无敌计时
├── test_collision.cpp         ← CollisionSystem 碰撞检测
├── test_game_model.cpp        ← SpriteEntityVM 集成测试
└── test_map_vm.cpp            ← GameMapVM 测试（后续）
```

---

## 5. 三绑定详细规范

### 5.1 定义

| # | 绑定类型 | 方向 | 实现方式 | 用途举例 |
|---|---|---|---|---|
| ① | **属性绑定** | ViewModel → View | `connect(VM::signal, View::slot)` | 分数变了→刷新显示 |
| ② | **命令绑定** | View → ViewModel | View 持有 `std::function` 引用并调用 | 按 W→向上移动 |
| ③ | **事件绑定** | ViewModel → View | `connect(VM::signal, View::slot)` | 游戏结束→弹出结束画面 |

### 5.2 属性绑定（单向数据流）

ViewModel 通过 signal 将数据变化推送给 View。View 在槽函数中更新 UI。

```cpp
// ViewModel 中（SpriteEntityVM.hpp）
signals:
    void scoreChanged(int newScore);

// AppAgent 中建立连接
connect(spriteVM, &SpriteEntityVM::scoreChanged,
        hudOverlay, &HudOverlay::onScoreChanged);

// View 的槽函数
void HudOverlay::onScoreChanged(int score) {
    scoreLabel->setText(QString("分数: %1").arg(score));
}
```

ViewModel 的完整信号列表（SpriteEntityVM）：

| 信号 | 参数 | 触发时机 |
|---|---|---|
| `scoreChanged` | `int` | 得分变化时 |
| `livesChanged` | `int` | 生命变化时 |
| `playerPosChanged` | 无 | 玩家位置变化时（每帧） |
| `enemiesChanged` | 无 | 敌机列表变化时（每帧） |
| `bulletsChanged` | 无 | 子弹列表变化时（每帧） |
| `gameStarted` | 无 | 调用 cmdStartGame 后 |
| `gameOver` | 无 | 玩家生命归零时 |

GameMapVM 的信号列表：

| 信号 | 参数 | 触发时机 |
|---|---|---|
| `scrollOffsetChanged` | `float` | 背景滚动偏移变化时 |

### 5.3 命令绑定

ViewModel 以公开的 `std::function` 成员变量暴露命令。View 通过指针引用调用。

```cpp
// ViewModel 中（SpriteEntityVM.hpp 公开成员）
class SpriteEntityVM : public QObject {
    Q_OBJECT
public:
    std::function<void()>             cmdStartGame;
    std::function<void(bool)>         cmdMoveUp;
    std::function<void(bool)>         cmdMoveDown;
    std::function<void(bool)>         cmdMoveLeft;
    std::function<void(bool)>         cmdMoveRight;
    std::function<void(float)>        cmdTick;
    std::function<void()>             cmdPause;
    std::function<void()>             cmdResume;

    SpriteEntityVM() {
        cmdStartGame = [this]() { startGameImpl(); };
        cmdMoveUp    = [this](bool a) { player_.moveUp(a); };
        cmdTick      = [this](float dt) { tickImpl(dt); };
        // ...
    }
};
```

GameMapVM 的命令：

```cpp
class GameMapVM : public QObject {
    Q_OBJECT
public:
    std::function<void(float)> cmdUpdate;
    // ...
};
```

**View 持有命令的方式：**
```cpp
// ✅ 正确
class GameView : public QGraphicsView {
    std::function<void(float)>* tickCmd_;
    std::function<void(bool)>*  moveUpCmd_;
public:
    GameView(std::function<void(float)>* tick,
             std::function<void(bool)>*  up, ...)
        : tickCmd_(tick), moveUpCmd_(up) {}

    void tick() { if (tickCmd_) (*tickCmd_)(0.016f); }

    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_W && moveUpCmd_) (*moveUpCmd_)(true);
    }
};

// ❌ 错误：直接调用 ViewModel 方法（绕过命令模式）
vm->startGame();  // 违反 MVVM 命令模式
```

### 5.4 事件绑定

ViewModel 的 signal 通知 View 做界面切换。

```cpp
// AppAgent 中
connect(spriteVM, &SpriteEntityVM::gameOver,
        gameView, &GameView::onGameOver);

// View 中
void GameView::onGameOver() {
    gameOverScreen->show();
    gameScene->setActive(false);
}
```

---

## 6. 命令模式规范

### 6.1 为什么用 std::function

> 课件原文：**"游戏类。使用 std::function 类实现命令模式。"**

不采用接口方式（ICommandBase + ICommandParameter）的原因：
- 不同方法签名需要定义不同的函数指针类型
- 序列化/反序列化方案在参数签名变化时**运行时才报错**，难以排查
- `std::function` 天然支持任意签名，**编译期类型安全**

### 6.2 命令定义规范

- 所有命令是 ViewModel 类的**公开成员变量**，类型为 `std::function`
- 命令名称以 `cmd` 前缀（如 `cmdStartGame`）
- 在 ViewModel 构造函数中用 lambda 绑定实现
- lambda 通过 `[this]` 捕获 ViewModel 指针，可直接访问私有成员

### 6.3 View 持有命令的规范

- View 通过**构造函数参数**接收命令指针（`std::function<...>*`）
- View **不能**直接调用 ViewModel 的私有方法
- View **不能**持有 ViewModel 的完整指针（只能持有命令指针）
- 命令指针由 AppAgent 在 `init()` 中传入

---

## 7. CMake 构建规则

### 7.1 OBJECT 库隔离编译

每个 Agent 编译成一个 **OBJECT 库**，最终统一链接到可执行文件。
编译时只能看到自己的头文件 + 依赖 Agent 的头文件，**编译阶段就防止越界 include**。

```cmake
# ▸ Common Agent — 不依赖任何人
add_library(thf_common OBJECT
    src/common/MathUtils.cpp
    src/common/Logger.cpp
    src/common/Geometry.cpp
)
target_include_directories(thf_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ▸ ViewModel Agent — 依赖 Common
add_library(thf_viewmodel OBJECT
    src/viewmodel/SpriteEntityVM.cpp
    src/viewmodel/GameMapVM.cpp
    src/viewmodel/Player.cpp
    src/viewmodel/Enemy.cpp
    src/viewmodel/Bullet.cpp
    src/viewmodel/CollisionSystem.cpp
    src/viewmodel/ScoreManager.cpp
)
target_include_directories(thf_viewmodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_viewmodel PUBLIC thf_common Qt5::Core)

# ▸ View Agent — 依赖 Common + ViewModel
add_library(thf_view OBJECT
    # View 的 .cpp 文件列表
)
target_include_directories(thf_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_view PUBLIC thf_common thf_viewmodel Qt5::Gui Qt5::Widgets)

# ▸ Resource Agent — 依赖 Common
add_library(thf_resource OBJECT
    src/resource/AssetManager.cpp
    src/resource/SaveManager.cpp
)
target_include_directories(thf_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_resource PUBLIC thf_common Qt5::Core Qt5::Gui)

# ▸ 主程序
add_executable(ThunderFighter
    src/app/main.cpp
    src/app/AppAgent.cpp
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
    $<TARGET_OBJECTS:thf_view>
    $<TARGET_OBJECTS:thf_resource>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    thf_common thf_viewmodel Qt5::Core Qt5::Gui Qt5::Widgets
)

# ▸ Tests — 依赖 Common + ViewModel（不依赖 View）
add_executable(ThunderFighter_tests
    tests/test_main.cpp
    tests/test_player.cpp
    tests/test_collision.cpp
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
)
target_link_libraries(ThunderFighter_tests PRIVATE
    thf_common thf_viewmodel Qt5::Core
    Catch2::Catch2WithMain
)
```

### 7.2 编译隔离效果

- ViewModel 想 `#include` View 的头文件 → **编译报错** ❌
- View 想 `#include` Enemy.hpp（数据类）→ **编译报错** ❌
- ViewModel 想 `#include <QWidget>` → **编译报错** ❌
- View 只能 include ViewModel 的公开头文件（SpriteEntityVM.hpp、GameMapVM.hpp）

### 7.3 QRC 资源文件

只包含图片资源，无 QML 文件：

```qrc
<RCC>
    <qresource prefix="/images">
        <file alias="playerShip">images/MyAircraft/Firepower.png</file>
        <file alias="enemySmall">images/PNG/Enemies/enemyRed3.png</file>
        <file alias="playerBullet">images/PNG/Lasers/laserBlue16.png</file>
        <file alias="background">images/Backgrounds/darkPurple.png</file>
    </qresource>
</RCC>
```

### 7.4 Debug 平台插件修复

MinGW Debug 构建时 Qt DLL 和平台插件可能版本不匹配，需要在 CMake 中添加后处理：

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND VCPKG_INSTALLED_DIR)
    set(_plugin_src "${VCPKG_INSTALLED_DIR}/x64-mingw-dynamic/debug/plugins/platforms/qwindows.dll")
    set(_plugin_dst "${CMAKE_BINARY_DIR}/bin/plugins/platforms/qwindows.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_plugin_src}" "${_plugin_dst}"
        COMMENT "Copying debug qwindows.dll platform plugin"
    )
endif()
```

---

## 8. Git 提交规则

### 8.1 基本原则

> **只在大范围重构后提交，小修改不提交。**

### 8.2 提交时机

| 场景 | 是否提交 |
|---|---|
| ViewModel Agent 核心逻辑完成 | ✅ 提交 |
| View Agent 完成 + 游戏可运行 | ✅ 提交 |
| 修了一个变量名拼写错误 | ❌ 不提交 |
| 调了一行颜色值 | ❌ 不提交 |
| 整个游戏可玩 Beta 版 | ✅ 提交 |

### 8.3 提交信息格式

```
<Agent-Name>: <概述>

<详细说明（可选）>
```

示例：
```
ViewModel: 实现玩家移动 + 碰撞检测

- Player 新增 moveUp/moveDown 方法
- CollisionSystem 支持子弹-敌机碰撞
- SpriteEntityVM 整合碰撞结果到 tickImpl()
```

### 8.4 不冲突保证

| Agent | 修改范围 | Git 冲突风险 |
|---|---|---|
| ① Common | `src/common/` + `include/common/` | 只改基础类型，极少冲突 |
| ② ViewModel | `src/viewmodel/` + `include/viewmodel/` | 与 View 不同目录，零冲突 |
| ③ View | `src/view/` + `include/view/` | 与 ViewModel 不同目录，零冲突 |
| ④ Resource | `src/resource/` + `include/resource/` | 独立，零冲突 |
| ⑤ App | `src/app/` + `include/app/` | 只做组装，改动最小 |
| ⑥ Test | `tests/` | 独立，零冲突 |

### 8.5 分支策略

- `main` — 稳定版本，只合入经过测试的大版本
- 开发过程中不需要开分支，直接在 `main` 上迭代

---

## 9. 常见违规检查清单

每个 Agent 在提交代码前，先自问：

| # | 问题 | Common | ViewModel | View | Resource | App | Test |
|---|---|---|---|---|---|---|---|
| 1 | 我 include 了不该 include 的 Agent 吗？ | — | 没 include View 吧？ | 没 include 数据类吧？ | 没 include ViewModel 吧？ | 没写业务逻辑吧？ | 没 include View 吧？ |
| 2 | 我直接操作游戏数据（Player/Enemy/Bullet）了吗？ | — | ✅ 这是本分 | 只通过 ViewModel 的命令/信号？ | — | — | ✅ |
| 3 | 我直接调用 ViewModel 的非命令方法了吗？ | — | — | 只通过 `std::function`？ | — | — | ✅ |
| 4 | 我直接读写磁盘文件了吗？ | — | 通过 Resource 了吗？ | 通过 Resource 了吗？ | ✅ 这是本分 | — | 用 Mock |
| 5 | 我引入了 QML/JS 了吗？ | — | — | **绝对禁止** | — | — | — |
| 6 | 我在 App Agent 里写逻辑/渲染代码了吗？ | — | — | — | — | **只做组装** | — |
| 7 | 我改了不属于我的目录吗？ | 仅 `common/` | 仅 `viewmodel/` | 仅 `view/` | 仅 `resource/` | 仅 `app/` | 仅 `tests/` |

> 每一条违规都可能破坏编译隔离和 MVVM 分层。**修改前先过一遍清单。** 🚀
