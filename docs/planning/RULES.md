# 雷霆战机 — 多智能体开发规则书 (RULES)

## 目录

1. [设计依据](#1-设计依据)
2. [开发环境](#2-开发环境)
3. [六个智能体总览](#3-六个智能体总览)
4. [各智能体详细规则](#4-各智能体详细规则)
5. [三绑定详细规范](#5-三绑定详细规范)
6. [命令模式规范](#6-命令模式规范)
7. [CMake 构建规则（编译隔离 + 统一链接）](#7-cmake-构建规则)
8. [Git 提交规则](#8-git-提交规则)
9. [常见违规检查清单](#9-常见违规检查清单)

---

## 1. 设计依据

本架构严格遵循课件 MVVM/MVFM 定义：

> **"ViewModel 层就是前述改进过的表现层。"**
> **"View 层和 ViewModel 层之间通过三个绑定来解开耦合：单向属性数据绑定、命令绑定、事件通知绑定。"**
> **"由于游戏类的数据即为可绘制对象，不需要转换，所以取消 Model 层，仅使用 ViewModel 层。"**
> **"可绘制对象有游戏地图和精灵图片两种，所以设计两个 ViewModel 类。"**
> **"使用 std::function 类实现命令模式。"**
> **"增加 viewmodel 子目录。"**

### 架构演进路线（参考课件 ex3→ex4→ex5）

```
ex3: 全部写在窗口类 → 膨胀，难以维护
ex4: MVC 拆分 → View 仍 include Model → 强耦合 ❌
ex5: MVVM 拆分 → View 完全不认识 ViewModel → 彻底解耦 ✅
```

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

### 2.2 可用 Qt 模块

| 模块 | 允许使用的 Agent |
|---|---|
| `Qt5::Core` | ViewModel, View, App, Resource, Test |
| `Qt5::Gui` | View 仅限（QPainter 渲染需要）；**SpiritVM 例外**（管理 QPixmap 精灵图片数据） |
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
| ① | **Common** | `src/common/` | 被两个及以上不同层共同使用的公共代码 | ❌ 不依赖任何人 |
| ② | **ViewModel** | `src/viewmodel/` | 游戏数据+规则，持有数据类对象，通过 const T* + std::function + signal 暴露 | 依赖 ① |
| ③ | **View** | `src/view/` | 纯 C++ QGraphicsView 渲染+按键捕获，**绝不认识 ViewModel** | 依赖 ① |
| ④ | **Resource** | `src/resource/` | 图片加载+存档读写（本质属于 ViewModel 层） | 依赖 ① |
| ⑤ | **App** | `src/app/` | 启动组装+建立三绑定连接，**唯一认识所有人的 Agent** | 依赖 ① ② ③ ④ |
| ⑥ | **Test** | `tests/` | 单元测试 Common 和 ViewModel | 依赖 ① ② |

**关键设计原则：**

```
依赖方向：Common ← ViewModel ── App ──→ View
                          ↕ （通过三绑定，不是代码依赖）
```

- View **绝对不能** `#include` ViewModel 的任何头文件
- ViewModel **绝对不能** `#include` View 的任何头文件
- ViewModel 和 View 之间的通信**全部通过 App 中转**的三绑定完成
- App 是**唯一的组装者**，负责 wire 三绑定

---

## 4. 各智能体详细规则

---

### ① Common Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/common/` + `include/common/` |
| **负责内容** | 枚举定义、常量、属性ID枚举——**仅放被两个及以上不同层使用的公共代码**。仅被一个层（如仅 ViewModel）使用的工具类不应放在 Common 层 |
| **可读（可 include）** | ❌ 无。不得 include 任何本项目其他文件 |
| **可写（可修改）** | 仅限自己目录下的文件 |
| **不可碰** | 任何其他 Agent 的文件 |

> **⚠️ Common 层核心原则：** Common 中的文件必须至少被两个不同的层使用。  
> ViewModel 层包含 Resource（参见下文 ViewModel 层规则），所以仅被 ViewModel + Resource 使用的文件**不算**两个层。  
> ✅ 示例：Actor.hpp / AirMap.hpp 被 View + ViewModel 使用 → 放在 Common 合理  
> ❌ 示例：MathUtils.hpp（Vec2）仅被 ViewModel 使用 → 应放在 `viewmodel/`

**文件清单：**
```
src/common/                     include/common/
├── Logger.cpp                  ├── Logger.hpp        ← log(tag, msg)（VM+App 使用）
├── AirMap.cpp                  ├── AirMap.hpp        ← 精灵集合（View+VM 使用）
                                ├── Actor.hpp          ← 精灵数据结构（View+VM 使用）
                                ├── Types.hpp          ← EntityType, GameState（View+VM 使用）
                                ├── Constants.hpp      ← SCREEN_WIDTH, PLAYER_SPEED（View+VM 使用）
                                └── PropertyIds.hpp    ← PROP_ID_MAP（View+VM 使用）
```

---

### ② ViewModel Agent — 核心层

| 规则项 | 内容 |
|---|---|
| **目录** | `src/viewmodel/` + `include/viewmodel/` |
| **负责内容** | 游戏全部数据和规则。采用 MVFM 思想，两个 ViewModel 类。命令用 `std::function`。聚合数据类（Player/Enemy/Bullet/CollisionSystem/ScoreManager 等）|
| **可读** | ✅ 可读 Common Agent<br>✅ **可读 Resource Agent**（Resource 本质属于 ViewModel 层）<br>❌ **不可读** View 的任何文件 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | `src/view/` — 绝不能 include 任何 UI 头文件 |
| **Qt 限制** | 可以 `#include <QObject>` 用于信号机制，**不能** include 任何 Widgets/Quick 头文件 |

> **📌 Resource 归属说明：** Resource Agent（AssetManager / SaveManager）本质属于 ViewModel 层。  
> 它提供图片加载和持久化服务，供 SpiritVM 和 GameMapVM 直接调用。  
> 因此 SpiritVM 可以直接 `#include "resource/AssetManager.hpp"` 并调用其方法。  
> **在判断 Common 层的"两个不同层"时，ViewModel 和 Resource 视为同一个层。**

**文件清单：**
```
src/viewmodel/                  include/viewmodel/
├── MathUtils.cpp               ├── MathUtils.hpp     ← Vec2, distance（从 common/ 移入）
├── Geometry.cpp                ├── Geometry.hpp      ← Rect, Circle（从 common/ 移入）
├── GameMapVM.cpp               ├── GameMapVM.hpp     ← 游戏地图 ViewModel（核心 FM）
├── SpiritVM.cpp                ├── SpiritVM.hpp      ← 精灵图片 ViewModel
├── Player.cpp                  ├── Player.hpp        ← 玩家数据类
├── Enemy.cpp                   ├── Enemy.hpp         ← 敌机数据类
├── Bullet.cpp                  ├── Bullet.hpp        ← 子弹数据类
├── CollisionSystem.cpp         ├── CollisionSystem.hpp ← 碰撞检测工具
├── ScoreManager.cpp            ├── ScoreManager.hpp  ← 计分 + 最高分
├── PowerUpManager.cpp          ├── PowerUpManager.hpp
├── AircraftStats.cpp           ├── AircraftStats.hpp
├── SkillSystem.cpp             ├── SkillSystem.hpp
├── WaveManager.cpp             ├── WaveManager.hpp
├── Boss.cpp                    ├── Boss.hpp
├── UpgradeManager.cpp          └── UpgradeManager.hpp
```

---

### ③ View Agent — 纯 C++（无 QML）

| 规则项 | 内容 |
|---|---|
| **目录** | `src/view/` + `include/view/` |
| **技术方案** | Qt5 `QGraphicsView` + `QGraphicsScene` + 自定义 `QGraphicsItem` 子类 |
| **负责内容** | QPainter 渲染画面、捕获键盘输入、管理界面切换。**纯 C++，无 QML/JS** |
| **可读** | ✅ 可读 Common Agent（包含数据类型和属性ID）<br>❌ **绝对不可读** ViewModel 的任何文件<br>❌ **不可读** Resource 的实现细节 |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ❌ **绝不能** `#include` 任何 `viewmodel/` 下的文件<br>❌ **绝不能**直接调用 ViewModel 的实例方法（只能通过 std::function 命令）<br>❌ **绝不能**直接操作游戏数据（Player/Enemy/Bullet 数据类）<br>❌ **绝不能**直接读写文件 |
| **持有的东西** | 只持有三种：<br>① `const T*` 属性指针（只读，来自 common/ 的数据类型）<br>② `std::function` 命令对象（由 App 在 init() 中注入）<br>③ `uint32_t` 属性 ID（在通知回调中判断哪个属性变化）|

> ⚠️ **这是最重要的规则**：View **完全不认识** ViewModel。View 头文件中**绝对不能**出现 `#include "../viewmodel/"` 或 `#include <viewmodel/>`。

**文件清单：**
```
src/view/                       include/view/
├── GameView.cpp                ├── GameView.hpp        ← QGraphicsView 主窗口
├── GameScene.cpp               ├── GameScene.hpp       ← QGraphicsScene 场景
├── PlayerItem.cpp              ├── PlayerItem.hpp      ← 玩家飞机
├── EnemyItem.cpp               ├── EnemyItem.hpp       ← 敌机
├── BulletItem.cpp              ├── BulletItem.hpp      ← 子弹
├── StarFieldItem.cpp           ├── StarFieldItem.hpp   ← 星空背景
├── HudOverlay.cpp              ├── HudOverlay.hpp      ← 分数/生命覆盖层
├── StartScreen.cpp             ├── StartScreen.hpp     ← 开始界面
├── GameOverScreen.cpp          ├── GameOverScreen.hpp  ← 游戏结束界面
├── AircraftSelectScreen.cpp    ├── AircraftSelectScreen.hpp  (后续)
├── ModeSelectScreen.cpp        ├── ModeSelectScreen.hpp      (后续)
├── BossHealthBar.cpp           ├── BossHealthBar.hpp         (后续)
├── PauseOverlay.cpp            ├── PauseOverlay.hpp          (后续)
├── StageClearScreen.cpp        ├── StageClearScreen.hpp      (后续)
├── UpgradeScreen.cpp           ├── UpgradeScreen.hpp         (后续)
└── ExplosionEffect.cpp         └── ExplosionEffect.hpp       (后续)
```

---

### ④ Resource Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/resource/` + `include/resource/` |
| **负责内容** | 加载 PNG 图片到 QPixmap（带缓存）、读写最高分/升级数据 JSON 文件 |
| **可读** | ✅ 可读 Common Agent<br>✅ **可被 ViewModel 读取**（Resource 本质属于 ViewModel 层）<br>❌ **不可读** View 的任何文件 |
| **可写** | 仅限自己目录下的文件 |

> **📌 Resource 与 ViewModel 的关系：** Resource 提供图片加载（AssetManager）和持久化（SaveManager）服务，  
> 供 SpiritVM 和 GameMapVM 直接调用。两者属于同一逻辑层。

```
src/resource/                   include/resource/
├── AssetManager.cpp            ├── AssetManager.hpp  ← getImage(key) → QPixmap
└── SaveManager.cpp             └── SaveManager.hpp   ← load/saveHighScore(int)
```

**关键规则**：ViewModel 和 View **绝对不能直接调用 QFile / QImage / QPixmap 读写文件**。所有文件 I/O 必须通过 Resource Agent。

---

### ⑤ App Agent

| 规则项 | 内容 |
|---|---|
| **目录** | `src/app/` + `include/app/` |
| **负责内容** | main 函数、创建所有 Agent 实例、建立三绑定连接、启动 QTimer 帧循环 |
| **技术方案** | `QApplication` + `QGraphicsView`，**无** QQmlApplicationEngine |
| **可读** | ✅ 可读所有 Agent（它是唯一认识所有人的） |
| **可写** | 仅限自己目录下的文件 |
| **不可碰** | ⚠️ **绝对不能**写任何游戏逻辑或渲染代码——只做组装和连接 |

```
src/app/                        include/app/
├── AppAgent.cpp                ├── AppAgent.hpp  ← init() / run() / shutdown()
└── main.cpp                    └── (无头文件)
```

**App 层附加规则（来自课件 FAQ + 实践）：**
- App 层**不能放入**应只属于 ViewModel/Model 内部的业务逻辑（如碰撞检测、敌机生成）
- App 层**不能包含** ViewModel 层的功能代码（如图片加载、数据初始化）。这些应归 SpiritVM 或 GameMapVM 自己处理
- App 层**可以**做跨 Agent 的轻量编排（如从存档读取升级数据后调用 `m_mapVM->initUpgradeData()`），因为 App 是唯一认识所有 Agent 的层
- App 层**只能**绑定 View 层的通知，**不能**绑定来自其他层的通知

> **📌 Part 类概念（课件 App 层拓展）：** 当项目复杂时，可将一套 View + ViewModel + Model 对象封装成 **Part 类**，以方便 App 管理生命周期。例如 `GamePart` 包含 GameView + GameMapVM + SpiritVM + Resource。App 持有 Part 类而非逐个持有各 Agent。Part 类本身也可以暴露 Command 供 View 调用（如创建新窗口的命令）。

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
├── test_game_map_vm.cpp       ← GameMapVM 集成测试
└── test_spirit_vm.cpp         ← SpiritVM 测试（后续）
```

---

## 5. 三绑定详细规范

### 5.1 定义

| # | 绑定类型 | 方向 | 实现方式 | 用途举例 |
|---|---|---|---|---|
| ① | **属性绑定** | ViewModel → View | View 持有 `const T*` 指针，在 paint() 中只读使用 | 地图数据更新 → 重绘 |
| ② | **命令绑定** | View → ViewModel | View 持有 `std::function`，调用时不知道实现者 | 按 W → 调用 moveUp 命令 |
| ③ | **事件绑定** | ViewModel → View | ViewModel 发出 signal/通知，App 中转给 View | 游戏结束 → 弹出结束画面 |

> **📌 PropertyTrigger 等价说明**：老师 ex5 中 ViewModel 继承 `PropertyTrigger`，通过 `fire(PROP_ID_MAP)` 发出通知。在 Qt 版本中等价为 `emit propertyChanged(PROP_ID_MAP)`。老师 ex5 中 View 通过 `add_notification(callback)` 注册回调，在 Qt 版本中等价为 `connect(vm, &VM::propertyChanged, view, &View::onPropertyChanged)`。**架构意义完全相同**。

### 5.2 属性绑定（单向数据流，const T* 指针）

ViewModel 通过 getter 方法暴露 `const T*` 指针。View 持有该指针，在 paint() 时读取。

```cpp
// ViewModel 中（GameMapVM.hpp）
class GameMapVM : public QObject {
    Q_OBJECT
public:
    const AirMap* getMap() const noexcept { return &m_map; }
    // 其他属性...
signals:
    void propertyChanged(uint32_t propertyId);
};

// AppAgent 中建立绑定
m_gameScene.setMap(m_gameMapVM.getMap());

// View 中（GameScene.hpp）— 不 include 任何 ViewModel 头文件
#include "common/air_map.h"     // ← 只 include common/ 的数据类型
class GameScene : public QGraphicsScene {
public:
    void setMap(const AirMap* map) noexcept { m_pMap = map; }
    void onPropertyChanged(uint32_t id) {
        if (id == PROP_ID_MAP) update();  // 收到通知后重绘
    }
protected:
    void paint() override {
        // 从 m_pMap 指针读取数据，绘制所有精灵
    }
private:
    const AirMap* m_pMap = nullptr;  // ← 只读指针，来自 ViewModel
};
```

**属性绑定的关键规则：**

- View 持有的指针类型为 `const T*`，**只读不可写**
- 数据类型的定义在 **common/** 目录下，View 和 ViewModel 都 include 它
- View 通过事件通知知道数据变化了，然后在 paint() 中重新读取指针

### 5.3 命令绑定（std::function）

ViewModel 以 `std::function` 形式暴露命令。View 持有命令并在用户操作时调用。

```cpp
// ViewModel 中（GameMapVM.hpp）
class GameMapVM : public QObject {
    Q_OBJECT
public:
    std::function<void(int)> getMoveUpCommand() {
        return [this](int type) { moveUpImpl(type); };
    }
    std::function<void(float)> getTickCommand() {
        return [this](float dt) { tickImpl(dt); };
    }
    // ...
private:
    void moveUpImpl(int type);  // 实际的实现
    void tickImpl(float dt);    // 实际的实现
};

// AppAgent 中注入命令
m_gameView.setMoveUpCommand(m_gameMapVM.getMoveUpCommand());
m_gameView.setTickCommand(m_gameMapVM.getTickCommand());

// View 中（GameView.hpp）— 不 include 任何 ViewModel
class GameView : public QGraphicsView {
public:
    void setMoveUpCommand(std::function<void(int)>&& cmd) { m_moveUpCmd = std::move(cmd); }
    void setTickCommand(std::function<void(float)>&& cmd) { m_tickCmd = std::move(cmd); }

    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_W && m_moveUpCmd) (*m_moveUpCmd)(MOVE_UP);
    }

    void tick() {
        if (m_tickCmd) (*m_tickCmd)(0.016f);
    }

private:
    std::function<void(int)>   m_moveUpCmd;   // 不知道谁实现的
    std::function<void(float)> m_tickCmd;      // 不知道谁实现的
};
```

**命令绑定的关键规则：**
- View **不能**直接调用 ViewModel 的实例方法（如 `vm->moveUp()`）
- View **只能**通过 `std::function` 命令对象间接调用
- 命令的 getter 方法命名以 `get` 开头，以 `Command` 结尾（如 `getMoveUpCommand`）

### 5.4 事件绑定（PropertyChanged 通知）

ViewModel 的数据变化时，发出通知。App 中转给 View。

```cpp
// ViewModel 中（GameMapVM.cpp）
void GameMapVM::tickImpl(float dt) {
    // ... 更新数据 ...
    emit propertyChanged(PROP_ID_MAP);     // 通知地图数据变了
    emit propertyChanged(PROP_ID_SCORE);   // 通知分数变了
}

// AppAgent 中建立连接
connect(&m_gameMapVM, &GameMapVM::propertyChanged,
        &m_gameScene, &GameScene::onPropertyChanged);

// View 中（GameScene.cpp）
void GameScene::onPropertyChanged(uint32_t id) {
    switch (id) {
    case PROP_ID_MAP:
        update();           // 重绘场景
        break;
    case PROP_ID_SCORE:
        m_pHud->updateScore(*m_pScore);
        break;
    case PROP_ID_GAME_STATE:
        showGameOverScreen();
        break;
    }
}
```

**事件绑定的关键规则：**
- App **只连接** ViewModel 到 View 的事件通知
- App **不能**连接其他跨层事件通知（如 Model→ViewModel，或 ViewModel→Resource）

---

## 6. 命令模式规范

### 6.1 为什么用 std::function

> 课件原文：**"使用 std::function 类实现命令模式。"**

课件对比了四种命令实现方式后，选择 `std::function`：
- 函数指针需要额外的上下文指针，不同签名需定义不同函数指针
- 接口方式（ICommandBase + ICommandParameter）参数变化时运行时才报错
- `std::any` + `std::tuple` 方式类型转换不一致时运行时崩溃
- `std::function` 含有函数签名，**编译期类型安全** ✅

### 6.2 命令定义规范

- 命令以 **getter 方法** 的形式从 ViewModel 暴露，命名：`getXxxCommand()`
- 命令类型为 `std::function<...>`
- 命令方法在 ViewModel 构造函数或命令 getter 中用 lambda 绑定实现
- lambda 通过 `[this]` 捕获 ViewModel 指针，调用私有实现方法

```cpp
// ✅ 正确
std::function<void(float)> getTickCommand() {
    return [this](float dt) { tickImpl(dt); };
}

// ❌ 错误 — std::function 作为公开成员变量（与老师 ex5 不一致）
// 老师 ex5 的 Plane 用 getter 方法返回，不是公开成员变量
```

### 6.3 View 持有命令的规范

- View 通过 **setter 方法** 接收命令，命名：`setXxxCommand(std::function<...>&&)`
- View **不能**持有 ViewModel 的完整指针（只能持有命令）
- 命令由 AppAgent 在 `init()` 中注入
- View 调用命令前需判空：`if (m_cmd) (*m_cmd)(params);`

---

## 7. CMake 构建规则

### 7.1 OBJECT 库隔离编译

每个 Agent 编译成一个 **OBJECT 库**，最终统一链接到可执行文件。
编译时只能看到自己的头文件 + 依赖 Agent 的头文件，**编译阶段就防止越界 include**。

```cmake
# ▸ Common Agent — 只放被两个及以上不同层使用的代码
# MathUtils / Geometry 仅 ViewModel 使用，放在 viewmodel/ 下
add_library(thf_common OBJECT
    src/common/Logger.cpp
    src/common/AirMap.cpp
)
target_include_directories(thf_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ▸ ViewModel Agent — 依赖 Common + Resource
# Resource 本质属于 ViewModel 层，thf_viewmodel 直接链接 thf_resource 以调用 AssetManager
add_library(thf_viewmodel OBJECT
    src/viewmodel/MathUtils.cpp
    src/viewmodel/Geometry.cpp
    src/viewmodel/GameMapVM.cpp
    src/viewmodel/SpiritVM.cpp
    src/viewmodel/Player.cpp
    src/viewmodel/Enemy.cpp
    src/viewmodel/Bullet.cpp
    src/viewmodel/CollisionSystem.cpp
    src/viewmodel/ScoreManager.cpp
    src/viewmodel/PowerUpManager.cpp
    src/viewmodel/AircraftStats.cpp
    src/viewmodel/SkillSystem.cpp
    src/viewmodel/WaveManager.cpp
    src/viewmodel/Boss.cpp
    src/viewmodel/UpgradeManager.cpp
)
target_include_directories(thf_viewmodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_viewmodel PUBLIC thf_resource thf_common Qt5::Core Qt5::Gui)

# ▸ View Agent — 只依赖 Common（绝不依赖 ViewModel！）
add_library(thf_view OBJECT
    # View 的 .cpp 文件列表
)
target_include_directories(thf_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_view PUBLIC thf_common Qt5::Gui Qt5::Widgets)
# ⚠️ 注意：thf_view 没有链接 thf_viewmodel！

# ▸ Resource Agent — 依赖 Common
add_library(thf_resource OBJECT
    src/resource/AssetManager.cpp
    src/resource/SaveManager.cpp
)
target_include_directories(thf_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_resource PUBLIC thf_common Qt5::Core Qt5::Gui)

# ▸ 主程序（App 链接所有）
add_executable(ThunderFighter
    src/app/main.cpp
    src/app/AppAgent.cpp
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
    $<TARGET_OBJECTS:thf_view>
    $<TARGET_OBJECTS:thf_resource>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    thf_common thf_viewmodel thf_view thf_resource
    Qt5::Core Qt5::Gui Qt5::Widgets
)

# ▸ Tests — 依赖 Common + ViewModel + Resource
add_executable(ThunderFighter_tests
    tests/test_main.cpp
    tests/test_player.cpp
    tests/test_collision.cpp
    tests/test_game_map_vm.cpp
    tests/test_aircraft_stats.cpp
    tests/test_skill_system.cpp
    tests/test_wave_manager.cpp
    tests/test_power_up.cpp
    tests/test_boss.cpp
    tests/test_upgrade_manager.cpp
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
    $<TARGET_OBJECTS:thf_resource>
)
target_link_libraries(ThunderFighter_tests PRIVATE
    thf_common thf_viewmodel thf_resource
    Qt5::Core
    Catch2::Catch2WithMain
)
```

### 7.2 编译隔离效果

- ViewModel 想 `#include` View 的头文件 → **编译报错** ❌
- View 想 `#include` ViewModel 的头文件 → **编译报错** ❌
- View 想 `#include` Player.hpp（数据类）→ **编译报错** ❌（因为数据类在 viewmodel/ 下）
- ViewModel 想 `#include <QWidget>` → **编译报错** ❌
- 只有 App 和 Test 可以 include ViewModel

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

MinGW Debug 构建时需要复制 Qt 平台插件：

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
| 整个迭代完成 | ✅ 提交 |

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
- GameMapVM 整合碰撞结果到 tickImpl()
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
|---|---|---|---|---|---|---|---|---|
| 1 | 我 include 了不该 include 的 Agent 吗？ | — | 没 include View 吧？ | **没 include viewmodel/ 吧？** | 没 include ViewModel 吧？ | 没写业务逻辑吧？ | 没 include View 吧？ |
| 2 | 我直接操作游戏数据了吗？ | — | ✅ 这是本分 | **绝不能！** 只读 const T* | — | — | ✅ |
| 3 | 我直接调用 ViewModel 实例方法了吗？ | — | — | **只能 std::function！** | — | — | ✅ |
| 4 | 我直接读写磁盘文件了吗？ | — | 通过 Resource | 通过 Resource | ✅ 本分 | — | 用 Mock |
| 5 | 我引入了 QML/JS 了吗？ | — | — | **绝对禁止** | — | — | — |
| 6 | 我在 App 里写逻辑/渲染代码了吗？ | — | — | — | — | **只做组装** | — |
| 7 | 我改了不属于我的目录吗？ | 仅 `common/` | 仅 `viewmodel/` | 仅 `view/` | 仅 `resource/` | 仅 `app/` | 仅 `tests/` |
| 8 | View 的 paint() 用 const T* 读数据了吗？ | — | — | ✅ **正确做法** | — | — | — |
| 9 | App 只绑定了 View 层的通知吗？ | — | — | — | — | **只绑定 View** | — |
| 10 | **Common 文件至少两个不同层用吗？** | ❌ **仅单层用不应在此** | 含 Resource 算同层 | — | 与 VM 同层 | — | — |
| 11 | **App 包含 ViewModel 功能吗？** | — | — | — | — | ❌ **加载等归 VM** | — |
| 12 | **Resource 被当作独立层吗？** | — | ❌ **Resource 算 VM** | — | ✅ **属于 VM 层** | — | — |

> **每一条违规都可能破坏编译隔离和 MVVM 分层。修改前先过一遍清单。** 🚀
