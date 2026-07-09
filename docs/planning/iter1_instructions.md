# 迭代 1 — 最小可玩版：Agent 指令

> **整体目标**：能"飞起来、打出去、撞上去、死掉重来"
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 和 `docs/planning/RULES.md`
>
> **核心约束**：View **绝对不能** include viewmodel/ 下的任何文件。所有通信通过三绑定。

---

## ① Common Agent — 公共基础设施

**目录**：`src/common/` + `include/common/`

**依赖**：❌ 不依赖任何人（不能 include 本项目任何其他文件）

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `include/common/Types.hpp` | 枚举 `EntityType`, `GameState`, `Direction` |
| `include/common/Constants.hpp` | 常量：屏幕 800×600、FPS 60、玩家/敌机/子弹参数 |
| `include/common/MathUtils.hpp` + `src/common/MathUtils.cpp` | `struct Vec2 { float x, y; }`，`distance()` |
| `include/common/Logger.hpp` + `src/common/Logger.cpp` | `log(tag, msg)` 简单打印 |
| `include/common/Geometry.hpp` + `src/common/Geometry.cpp` | `struct Rect`, `struct Circle`, `overlaps()` |
| `include/common/Actor.hpp` | `struct Actor { ActorType type; float x, y; int hp; int maxHp; }` |
| `include/common/AirMap.hpp` + `src/common/AirMap.cpp` | `class AirMap` — `vector<Actor>` 容器，提供 `clear/size/getAt/append` |
| `include/common/PropertyIds.hpp` | 属性 ID 枚举：`PROP_ID_MAP, SCORE, LIVES, GAME_STATE, MAP_OFFSET` |

### 关键约束

- **不能** include 本项目任何其他文件
- Actor.hpp 和 AirMap.hpp 是 View 唯一能看到的"数据窗口"
- PropertyIds.hpp 使用 `enum { ... }`（C 风格枚举，对齐老师）

### 完成后可验证

```cpp
// Types.hpp 至少包含
enum class GameState { Menu, Playing, GameOver };

// Constants.hpp 至少包含
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
constexpr float PLAYER_SPEED = 0.4f;
constexpr int PLAYER_MAX_LIVES = 3;

// Actor.hpp 包含
enum class ActorType { Player, EnemySmall, PlayerBullet, EnemyBullet };
struct Actor { ActorType type; float x, y; int hp; int maxHp; };

// AirMap 至少提供
class AirMap {
    void clear();
    size_t size() const;
    const Actor& getAt(size_t idx) const;
    void append(const Actor& actor);
};

// PropertyIds.hpp 至少包含
enum { PROP_ID_MAP = 1, PROP_ID_SCORE, PROP_ID_LIVES, PROP_ID_GAME_STATE };
```

---
---

## ② ViewModel Agent — 核心游戏逻辑

**目录**：`src/viewmodel/` + `include/viewmodel/`

**依赖**：✅ 可读 Common Agent | ❌ 不可读 View / Resource

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `include/viewmodel/Player.hpp` + `src/viewmodel/Player.cpp` | 玩家：位置、生命、移动、射击、无敌计时 |
| `include/viewmodel/Enemy.hpp` + `src/viewmodel/Enemy.cpp` | 敌机：位置、速度、生命、直线下飞 |
| `include/viewmodel/Bullet.hpp` + `src/viewmodel/Bullet.cpp` | 子弹：位置、速度、归属(玩家/敌方) |
| `include/viewmodel/CollisionSystem.hpp` + `src/viewmodel/CollisionSystem.cpp` | 静态工具：子弹vs敌机、敌机vs玩家 |
| `include/viewmodel/ScoreManager.hpp` + `src/viewmodel/ScoreManager.cpp` | 计分、最高分管理 |
| `include/viewmodel/GameMapVM.hpp` + `src/viewmodel/GameMapVM.cpp` | **核心** — 聚合所有数据，继承 QObject，暴露三绑定 |
| `include/viewmodel/SpiritVM.hpp` + `src/viewmodel/SpiritVM.cpp` | 加载精灵图片（QPixmap），暴露 const 指针 |

### GameMapVM 的核心结构（对齐 ex5 GameViewModel）

```cpp
class GameMapVM : public QObject {
    Q_OBJECT
public:
    explicit GameMapVM(QObject* parent = nullptr);
    void reset();  // 重置游戏状态

    // ① 属性暴露 — 返回 const 指针
    const AirMap* getMap() const noexcept;
    int getScore() const noexcept;
    int getLives() const noexcept;
    int getHighScore() const noexcept;
    GameState getGameState() const noexcept;

    // ② 命令暴露 — 返回 std::function（对齐 ex5 的 get_next_step_command）
    std::function<void()>         getStartGameCommand();
    std::function<void(int)>      getMoveUpCommand();    // int = 1(按下) / 0(松开)
    std::function<void(int)>      getMoveDownCommand();
    std::function<void(int)>      getMoveLeftCommand();
    std::function<void(int)>      getMoveRightCommand();
    std::function<void(float)>    getTickCommand();      // float = deltaTime

signals:
    void propertyChanged(uint32_t propertyId);  // 对齐 ex5 的 fire(PROP_ID_XXX)

private:
    void startGameImpl();
    void tickImpl(float dt);
    void moveUpImpl(int active);
    void syncMap();  // 将内部数据同步到 AirMap

    Player m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<Bullet> m_bullets;
    ScoreManager m_scoreMgr;
    AirMap m_map;
    // ...
};
```

### SpiritVM 的结构（对齐 ex5 SpiritViewModel）

```cpp
class SpiritVM {
public:
    const QPixmap* getPlayerPixmap() const noexcept;
    const QPixmap* getEnemySmallPixmap() const noexcept;
    const QPixmap* getPlayerBulletPixmap() const noexcept;
    const QPixmap* getBackgroundPixmap() const noexcept;
    bool initialize();  // 从 QRC 加载图片
private:
    QPixmap m_playerImg;
    QPixmap m_enemySmallImg;
    QPixmap m_playerBulletImg;
    QPixmap m_bgImg;
};
```

### tickImpl() 的执行顺序（对齐 ex5 GameViewModel::next_step）

```
1. Player::update(dt)          ← 移动、无敌计时递减
2. 自动射击（Player::canFire） → 生成 Bullet
3. 敌机生成（定时）            → 生成 Enemy
4. Enemy::update(dt)           ← 移动
5. Bullet::update(dt)          ← 飞行
6. CollisionSystem::check()    ← 碰撞处理
7. 清理死亡/离屏实体
8. syncMap()                   ← 同步到 AirMap
9. emit propertyChanged(...)   ← 通知 View
```

### 关键约束

- **不能** include View 的任何头文件
- **不能** include `<QWidget>` / `<QGraphicsView>` 等 UI 头文件
- 只能 `#include <QObject>` 和 QtCore 头文件
- 命令通过 getter 方法返回 `std::function`（不是公开成员变量）
- 数据变化后调用 `emit propertyChanged(PROP_ID_XXX)`

---
---

## ③ View Agent — 纯 C++ 渲染层

**目录**：`src/view/` + `include/view/`

**依赖**：✅ 可读 Common Agent | ❌ **绝不能** include viewmodel/ 下的任何文件

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `include/view/GameView.hpp` + `src/view/GameView.cpp` | 主窗口 QGraphicsView，拥有 QTimer 驱动帧循环 |
| `include/view/GameScene.hpp` + `src/view/GameScene.cpp` | QGraphicsScene，持有 `const AirMap*`，paint() 遍历绘制 |
| `include/view/PlayerItem.hpp` + `src/view/PlayerItem.cpp` | 玩家飞机 QGraphicsPixmapItem |
| `include/view/EnemyItem.hpp` + `src/view/EnemyItem.cpp` | 敌机 QGraphicsPixmapItem |
| `include/view/BulletItem.hpp` + `src/view/BulletItem.cpp` | 子弹 QGraphicsPixmapItem |
| `include/view/StarFieldItem.hpp` + `src/view/StarFieldItem.cpp` | 星空背景（QGraphicsItem 自定义 paint） |
| `include/view/HudOverlay.hpp` + `src/view/HudOverlay.cpp` | HUD（分数/生命显示） |
| `include/view/StartScreen.hpp` + `src/view/StartScreen.cpp` | 开始界面（标题 + 开始按钮） |
| `include/view/GameOverScreen.hpp` + `src/view/GameOverScreen.cpp` | 游戏结束界面（分数 + 再来一局） |

### GameView 的核心结构

```cpp
class GameView : public QGraphicsView {
public:
    explicit GameView(QWidget* parent = nullptr);

    // 属性绑定 — 接收数据指针（从 App 注入）
    void setMap(const AirMap* map) noexcept;
    void setPlayerPixmap(const QPixmap* p) noexcept;
    void setEnemySmallPixmap(const QPixmap* p) noexcept;
    void setBulletPixmap(const QPixmap* p) noexcept;
    void setBackgroundPixmap(const QPixmap* p) noexcept;

    // 命令绑定 — 接收命令（从 App 注入）
    void setTickCommand(std::function<void(float)>&& cmd);
    void setMoveUpCommand(std::function<void(int)>&& cmd);
    void setMoveDownCommand(std::function<void(int)>&& cmd);
    void setMoveLeftCommand(std::function<void(int)>&& cmd);
    void setMoveRightCommand(std::function<void(int)>&& cmd);
    void setStartGameCommand(std::function<void()>&& cmd);

    // 事件绑定 — 接收 ViewModel 通知
    void onPropertyChanged(uint32_t propertyId);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;  // fitInView

private:
    QTimer* m_timer;  // 帧循环（对齐 ex5 MainWindow 的 Fl::add_timeout）
    GameScene* m_scene;
    // 属性指针 — 全是 const T*
    const AirMap* m_pMap = nullptr;
    const QPixmap* m_pPlayerImg = nullptr;
    // ...
    // 命令 — 全是 std::function
    std::function<void(float)> m_tickCommand;
    std::function<void(int)> m_moveUpCommand;
    // ...
};
```

### 帧循环（对齐 ex5 MainWindow::timeout_cb）

```cpp
GameView::GameView(QWidget* parent) : QGraphicsView(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        if (m_tickCommand) m_tickCommand(0.016f);
    });
    m_timer->start(16);  // ~60 FPS
}
```

### 键盘处理

```cpp
void GameView::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_W: case Qt::Key_Up:    if (m_moveUpCommand)    (*m_moveUpCommand)(1); break;
    case Qt::Key_S: case Qt::Key_Down:  if (m_moveDownCommand)  (*m_moveDownCommand)(1); break;
    case Qt::Key_A: case Qt::Key_Left:  if (m_moveLeftCommand)  (*m_moveLeftCommand)(1); break;
    case Qt::Key_D: case Qt::Key_Right: if (m_moveRightCommand) (*m_moveRightCommand)(1); break;
    }
}
void GameView::keyReleaseEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_W: case Qt::Key_Up:    if (m_moveUpCommand)    (*m_moveUpCommand)(0); break;
    case Qt::Key_S: case Qt::Key_Down:  if (m_moveDownCommand)  (*m_moveDownCommand)(0); break;
    case Qt::Key_A: case Qt::Key_Left:  if (m_moveLeftCommand)  (*m_moveLeftCommand)(0); break;
    case Qt::Key_D: case Qt::Key_Right: if (m_moveRightCommand) (*m_moveRightCommand)(0); break;
    }
}
```

### 屏幕适应

```cpp
void GameView::resizeEvent(QResizeEvent* e) {
    QGraphicsView::resizeEvent(e);
    fitInView(0, 0, 800, 600, Qt::KeepAspectRatio);
}
```

### 通知处理（对齐 ex5 MainWindow::get_notification）

```cpp
void GameView::onPropertyChanged(uint32_t id) {
    switch (id) {
    case PROP_ID_MAP:
        m_scene->update();  // 重绘所有精灵
        break;
    case PROP_ID_SCORE:
        m_hud->updateScore();  // 从 m_pScore 指针读取新分数
        break;
    case PROP_ID_LIVES:
        m_hud->updateLives();
        break;
    case PROP_ID_GAME_STATE:
        // 切换界面（App 负责监听 state 切换 QStackedWidget）
        break;
    }
}
```

### 界面切换

由 App 的 QStackedWidget 控制，但 View 层面：

```
GameView 构造函数创建所有界面 QWidget：
  m_pageStack->addWidget(m_startScreen);     // 页面 0
  m_pageStack->addWidget(m_gameWidget);      // 页面 1（含 scene）
  m_pageStack->addWidget(m_gameOverScreen);  // 页面 2

onPropertyChanged 中切换：
  case PROP_ID_GAME_STATE:
      switch (m_pGameState) {
          case Menu:    m_pageStack->setCurrentIndex(0); break;
          case Playing: m_pageStack->setCurrentIndex(1); break;
          case GameOver:m_pageStack->setCurrentIndex(2); break;
      }
```

### 关键约束

- **绝对不能** `#include` 任何 `viewmodel/` 下的文件
- 只能 include `common/` 下的文件（Actor.hpp, AirMap.hpp, PropertyIds.hpp, Constants.hpp 等）
- 仅能通过 `const T*` 指针读取数据（不能修改）
- 仅能通过 `std::function` 命令触发操作
- **无 QML，无 JS**，纯 C++ QPainter 绘制
- 界面切换用 `QStackedWidget`

---
---

## ④ Resource Agent — 资源与持久化

**目录**：`src/resource/` + `include/resource/`

**依赖**：✅ 可读 Common | ❌ 不可读 ViewModel / View

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `include/resource/AssetManager.hpp` + `src/resource/AssetManager.cpp` | 单例，`getImage(key)` → `QPixmap`，从 QRC 加载 + 缓存 |
| `include/resource/SaveManager.hpp` + `src/resource/SaveManager.cpp` | 最高分 JSON 存档的读写 |

### AssetManager 核心接口

```cpp
class AssetManager {
public:
    static AssetManager& instance();
    const QPixmap* getImage(const QString& key);  // 从 QRC 加载
private:
    std::map<QString, QPixmap> m_cache;
};
```

### SaveManager 核心接口

```cpp
class SaveManager {
public:
    SaveManager();
    int loadHighScore();
    void saveHighScore(int score);
private:
    QString m_filePath;  // AppData/save.json
};
```

### 关键约束

- SaveManager 是**唯一**读写磁盘文件的类
- 存档格式为 JSON

---
---

## ⑤ App Agent — 组装者

**目录**：`src/app/` + `include/app/`

**依赖**：✅ 可读所有 Agent | ⚠️ 只做组装，不写业务逻辑

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `include/app/AppAgent.hpp` + `src/app/AppAgent.cpp` | 创建所有实例，建立三绑定连接 |
| `src/app/main.cpp` | 入口：QApplication → AppAgent::init() → run() |

### AppAgent::init() 的核心逻辑（对齐 ex5 AirApp::initialize）

```cpp
bool AppAgent::initialize() {
    // 1. 创建所有 Agent
    m_gameMapVM = new GameMapVM(this);
    m_spiritVM = new SpiritVM();
    if (!m_spiritVM->initialize()) return false;
    m_gameView = new GameView();  // GameView 自建 QTimer

    // 2. ① 属性绑定 — 数据指针注入 View
    m_gameView->setMap(m_gameMapVM->getMap());
    m_gameView->setPlayerPixmap(m_spiritVM->getPlayerPixmap());
    m_gameView->setEnemySmallPixmap(m_spiritVM->getEnemySmallPixmap());
    m_gameView->setBulletPixmap(m_spiritVM->getPlayerBulletPixmap());
    m_gameView->setBackgroundPixmap(m_spiritVM->getBackgroundPixmap());

    // 3. ② 命令绑定 — 命令注入 View
    m_gameView->setTickCommand(m_gameMapVM->getTickCommand());
    m_gameView->setMoveUpCommand(m_gameMapVM->getMoveUpCommand());
    m_gameView->setMoveDownCommand(m_gameMapVM->getMoveDownCommand());
    m_gameView->setMoveLeftCommand(m_gameMapVM->getMoveLeftCommand());
    m_gameView->setMoveRightCommand(m_gameMapVM->getMoveRightCommand());
    m_gameView->setStartGameCommand(m_gameMapVM->getStartGameCommand());

    // 4. ③ 事件绑定 — ViewModel 通知 → View
    connect(m_gameMapVM, &GameMapVM::propertyChanged,
            m_gameView, &GameView::onPropertyChanged);

    return true;
}
```

### main.cpp

```cpp
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    AppAgent agent;
    if (!agent.initialize()) return 1;
    agent.show();  // 显示主窗口
    return app.exec();
}
```

### 关键约束

- **不能写任何游戏逻辑**（不能调 Player::takeDamage 等）
- **不能写任何渲染代码**（不能调 QPainter 画图）
- 只做三件事：创建实例 → 属性注入 → 命令注入 → 信号连接
- 帧循环在 GameView 中，App 不需要管

---
---

## ⑥ Test Agent — 单元测试

**目录**：`tests/`

**依赖**：✅ 可读 Common + ViewModel | ❌ 不可读 View

### 需要创建的文件

| 文件 | 内容 |
|---|---|
| `tests/test_main.cpp` | Catch2 主入口（Catch2WithMain 已提供 main） |
| `tests/test_player.cpp` | Player 移动/生命/无敌计时测试 |
| `tests/test_collision.cpp` | CollisionSystem 碰撞检测测试 |
| `tests/test_game_map_vm.cpp` | GameMapVM 集成测试 |

### 测试用例示例

```cpp
// test_player.cpp
TEST_CASE("Player starts with 3 lives", "[player]") {
    Player p;
    CHECK(p.getLives() == 3);
    CHECK(p.isDead() == false);
}

TEST_CASE("Player takes damage and becomes invincible", "[player]") {
    Player p;
    p.takeDamage();
    CHECK(p.getLives() == 2);
    CHECK(p.isInvincible() == true);
}

// test_collision.cpp
TEST_CASE("Bullet hits enemy", "[collision]") {
    Bullet b(0.5f, 0.5f, 0, -1, Bullet::Player);
    auto enemy = std::make_unique<Enemy>(0.5f, 0.5f, 0.25f);
    std::vector<Bullet> bullets = {b};
    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(std::move(enemy));
    auto hits = CollisionSystem::checkBulletEnemy(bullets, enemies);
    CHECK(hits.size() == 1);
}

// test_game_map_vm.cpp
TEST_CASE("GameMapVM reset starts game", "[gamemap]") {
    GameMapVM vm;
    vm.getStartGameCommand()();  // 调用 start 命令
    CHECK(vm.getGameState() == GameState::Playing);
    CHECK(vm.getLives() == 3);
}
```

### 关键约束

- **不能** include View 的任何文件
- **不启动 Qt 窗口**（使用 Qt::Core 即可，不需要 Gui/Widgets）
- 直接测试 ViewModel 的数据类和 GameMapVM

---
---

## 适配 CMakeLists.txt

所有 Agent 完成后，需要在 `CMakeLists.txt` 中添加：

```cmake
# Common
add_library(thf_common OBJECT
    src/common/MathUtils.cpp
    src/common/Logger.cpp
    src/common/Geometry.cpp
    src/common/AirMap.cpp
)
target_include_directories(thf_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ViewModel
add_library(thf_viewmodel OBJECT
    src/viewmodel/GameMapVM.cpp
    src/viewmodel/SpiritVM.cpp
    src/viewmodel/Player.cpp
    src/viewmodel/Enemy.cpp
    src/viewmodel/Bullet.cpp
    src/viewmodel/CollisionSystem.cpp
    src/viewmodel/ScoreManager.cpp
)
target_include_directories(thf_viewmodel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_viewmodel PUBLIC thf_common Qt5::Core)

# View — ⚠️ 不链接 thf_viewmodel！
add_library(thf_view OBJECT
    src/view/GameView.cpp
    src/view/GameScene.cpp
    src/view/PlayerItem.cpp
    # ... 其他 .cpp
)
target_include_directories(thf_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_view PUBLIC thf_common Qt5::Gui Qt5::Widgets)

# Resource
add_library(thf_resource OBJECT
    src/resource/AssetManager.cpp
    src/resource/SaveManager.cpp
)
target_include_directories(thf_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(thf_resource PUBLIC thf_common Qt5::Core Qt5::Gui)

# 主程序
add_executable(ThunderFighter
    src/app/main.cpp src/app/AppAgent.cpp
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
    $<TARGET_OBJECTS:thf_view>
    $<TARGET_OBJECTS:thf_resource>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    thf_common thf_viewmodel thf_view thf_resource
    Qt5::Core Qt5::Gui Qt5::Widgets
)

# Tests
add_executable(ThunderFighter_tests
    tests/test_main.cpp tests/test_player.cpp
    tests/test_collision.cpp tests/test_game_map_vm.cpp
    $<TARGET_OBJECTS:thf_common>
    $<TARGET_OBJECTS:thf_viewmodel>
)
target_link_libraries(ThunderFighter_tests PRIVATE
    thf_common thf_viewmodel Qt5::Core
    Catch2::Catch2WithMain
)
```

---

## 开发顺序建议

```
① Common  ═══════ 最先完成（其他人需要它的类型和工具）
     ↓
② ViewModel ═════ 第二（核心逻辑，Test 已可开始写测试）
     ↓
④ Resource ══════ 可与 View 并行（AssetManager 加载图片）
     ↓
③ View  ════════ 第三（需要 Common 的 Actor/AirMap，不需要 ViewModel）
     ↓
⑤ App  ═════════ 最后（所有 Agent 完成后，组装三绑定）
     ↓
⑥ Test  ════════ 从第二步开始就可以并行写测试
```

**注意**：各自开发时互不依赖头文件——View 只需要 `common/` 下的文件，ViewModel 只需要 `common/` 下的文件，他们可以**完全并行开发**。这就是老师说的"View 层和 ViewModel 层彻底解开了耦合"。
