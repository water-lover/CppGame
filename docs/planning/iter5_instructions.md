# 迭代 5 — 5 战机 + 技能系统：Agent 指令

> **整体目标**：实现战机选择界面（AircraftSelectScreen），5 架战机可选，技能释放和 HUD 显示
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 迭代 5 节和 `docs/planning/RULES.md`
>
> **状态标记**：✅ = 已完成（无需再动） ❌ = 待完成

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

**无需修改**（Types/PropertyIds 已在迭代 3 中满足需求：`PROP_ID_SKILL_COOLDOWN`、`PROP_ID_WEAPON_LEVEL` 已存在）。

---

## ② ViewModel Agent

**目录**：`src/viewmodel/` + `include/viewmodel/`

### 已完成 ✅

| 文件 | 已实现内容 |
|---|---|
| `AircraftStats.hpp/cpp` | 5 架战机属性模板（雷霆/烈焰/冰霜/幻影/堡垒），各含火力/生命/速度/射击间隔/技能/冷却 |
| `SkillSystem.hpp/cpp` | 技能冷却管理（`init/update/activate`），状态查询（`isOnCooldown/getCooldownPercent/isActive`） |
| `GameMapVM.hpp` | `getSelectAircraftCommand()`、`getUseSkillCommand()`、`selectAircraftImpl()`、`useSkillImpl()`、`getAircraftType()`、`getAircraftName()`、`getWeaponLevel()`、`getSkillCooldownPercent()`、`isSkillReady()`、`isSkillActive()` |
| `GameMapVM.cpp` | `tickImpl()` 中调用 `m_skill.update(dt)`、`applySkillEffects()`；技能效果（全屏雷击/火焰风暴/时空闪避/护盾/铁壁反弹）已实现 |
| `SpiritVM.hpp` | `setAircraftPixmap(AircraftType, const QPixmap*)` 和 `getAircraftPixmap(AircraftType)` |

### 需要扩展的文件 ❌

#### `GameMapVM.hpp/cpp` — 新增属性暴露供 HUD 显示

**更改 1：新增选中的战机信息查询（已有，确认即可）**

```cpp
// ✅ 以下 getter 已有，确认可用：
int     getAircraftType()          const noexcept;  // 当前战机类型
const char* getAircraftName()      const noexcept;  // 战机名称
int     getWeaponLevel()           const noexcept;  // 武器等级 1~5
float   getSkillCooldownPercent()  const noexcept;  // 技能冷却 0~1
bool    isSkillReady()             const noexcept;  // 技能是否就绪
bool    isSkillActive()            const noexcept;  // 技能是否激活中
```

---

## ③ View Agent（主力）

**目录**：`src/view/` + `include/view/`

### 需要创建的文件 ❌

| 文件 | 内容 |
|---|---|
| `include/view/AircraftSelectScreen.hpp` | 战机选择界面头文件 |
| `src/view/AircraftSelectScreen.cpp` | 战机选择界面实现 |

### 需要修改的文件 ❌

| 文件 | 变更 |
|---|---|
| `include/view/GameView.hpp` | 新增 `AircraftSelectScreen` 页面 + 命令 setter + Space 键技能 |
| `src/view/GameView.cpp` | 构造函数创建 AircraftSelectScreen；页面栈重排；Space 键绑定；HUD 技能 CD 显示 |
| `include/view/HudOverlay.hpp` | 新增技能冷却指示器接口 |
| `src/view/HudOverlay.cpp` | 绘制技能冷却条/图标 |
| `include/view/GameScene.hpp` | 新增技能状态绘制接口 |
| `src/view/GameScene.cpp` | `drawForeground` 中绘制技能激活特效 |
| `CMakeLists.txt` | 添加新文件 |

---

### AircraftSelectScreen 设计

**界面布局（百分比定位，适应窗口缩放）：**

```
┌──────────────────────────────────────────────────┐
│                                                    │
│              选 择 你 的 战 机                      │
│                                                    │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│  │  雷霆号   │  │  烈焰号   │  │  冰霜号   │         │
│  │  ★★★☆☆  │  │  ★★★★★  │  │  ★★☆☆☆  │         │
│  │  ♥♥♥     │  │  ♥♥      │  │  ♥♥♥♥♥  │         │
│  │  均衡型   │  │  高火力   │  │  高血量   │         │
│  │ [选择]   │  │ [选择]   │  │ [选择]   │         │
│  └──────────┘  └──────────┘  └──────────┘         │
│                                                    │
│  ┌──────────┐  ┌──────────┐                        │
│  │  幻影号   │  │  堡垒号   │                        │
│  │  ★★★☆☆  │  │  ★★☆☆☆  │                        │
│  │  ♥♥      │  │  ♥♥♥♥   │                        │
│  │  极速     │  │  坦克     │                        │
│  │ [选择]   │  │ [选择]   │                        │
│  └──────────┘  └──────────┘                        │
│                                                    │
│  [确 认 选 择]                                      │
└──────────────────────────────────────────────────┘
```

**核心机制：**
- 显示 5 架战机的属性：名称、火力星级、生命、特长描述
- 玩家点击一架战机 → 高亮选中（`setSelectAircraftCommand(type)`）
- 底部"确认选择"按钮 → emit `confirmed()` 信号，进入模式选择
- 无需图片渲染，纯文字 + CSS 样式即可

**头文件接口：**

```cpp
// AircraftSelectScreen.hpp
#ifndef AIRCRAFTSTATESCREEN_HPP
#define AIRCRAFTSTATESCREEN_HPP

#include <QWidget>
#include <QPushButton>
#include <QVector>

/// 战机选择界面 — 5 架战机可选
class AircraftSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit AircraftSelectScreen(QWidget* parent = nullptr);
    ~AircraftSelectScreen() override = default;

    /// 注入"选择战机"命令（由 App 传递）
    void setSelectAircraftCommand(std::function<void(int)>&& cmd);

signals:
    /// 用户点击"确认选择"
    void confirmed();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct AircraftCardInfo {
        int id;               // AircraftType 枚举值
        const char* name;
        int firePower;        // 火力星级 1~5
        int lives;            // 生命
        const char* desc;     // 特长描述（如"均衡型"）
    };

    static const AircraftCardInfo AIRCRAFT[5];

    void setupUI();
    QPushButton* createAircraftCard(const AircraftCardInfo& info);
    void updateSelection(int selectedId);

    std::function<void(int)> m_selectAircraftCommand;
    QVector<QPushButton*> m_cards;
    QPushButton* m_confirmBtn = nullptr;
    int m_selectedIndex = 0;  // 默认选中雷霆号
};

#endif // AIRCRAFTSTATESCREEN_HPP
```

**飞机信息表：**

```cpp
const AircraftCardInfo AircraftSelectScreen::AIRCRAFT[5] = {
    {0, "雷 霆 号", 3, 3, "均衡型"},
    {1, "烈 焰 号", 5, 2, "高火力"},
    {2, "冰 霜 号", 2, 5, "高血量"},
    {3, "幻 影 号", 3, 2, "极速"},
    {4, "堡 垒 号", 2, 4, "坦克"},
};
```

---

### GameView 改动

#### 页面栈重排

当前页面索引：
```
  0: StartScreen
  1: ModeSelectScreen
  2: LevelSelectScreen
  3: GamePage
  4: GameOverScreen
  5: PauseOverlay
```

新页面索引（**AircraftSelectScreen 插入到 index 1，后续所有索引 +1**）：
```
  0: StartScreen
  1: AircraftSelectScreen    ← 新增（插入）
  2: ModeSelectScreen        ← 原 1
  3: LevelSelectScreen       ← 原 2
  4: GamePage                ← 原 3
  5: GameOverScreen          ← 原 4
  6: PauseOverlay            ← 原 5
```

**注意更新所有页面引用**：
- `GameState::Menu` → index 0
- `GameState::AircraftSelect` → index 1 ← 新增
- `GameState::ModeSelect` → index 2
- `GameState::LevelSelect` → index 3
- `GameState::Playing` → index 4
- `GameState::GameOver` → index 5
- `GameState::Paused` → index 6

#### 新增命令 setter

```cpp
// GameView.hpp
void setSelectAircraftCommand(std::function<void(int)>&& cmd);
void setUseSkillCommand(std::function<void()>&& cmd);
```

#### 构造函数新增连线

```cpp
// 开始界面 → 战机选择
connect(m_startScreen, &StartScreen::startClicked, [this]() {
    m_pageStack->setCurrentIndex(1);  // AircraftSelectScreen
});

// 战机选择 → 确认 → 模式选择
connect(m_aircraftSelectScreen, &AircraftSelectScreen::confirmed, [this]() {
    m_pageStack->setCurrentIndex(2);  // ModeSelectScreen
});
```

#### Space 键释放技能

```cpp
// GameView::keyPressEvent 新增分支
case Qt::Key_Space:
    if (m_useSkillCommand) m_useSkillCommand();
    break;
```

#### Types.hpp 新增 GameState::AircraftSelect

**需要在 Common 中新增**：在 `GameState` 枚举中增加 `AircraftSelect`（介于 `Menu` 和 `ModeSelect` 之间）。

---

### HUD 技能冷却显示

在 HudOverlay 中新增技能冷却条/图标：

```cpp
// HudOverlay.hpp — 新增
void setSkillCooldownPtr(const float* p) noexcept;
void setSkillReadyPtr(const bool* p) noexcept;
void setSkillActivePtr(const bool* p) noexcept;
void setAircraftNamePtr(const char* p) noexcept;
```

```cpp
// HudOverlay.cpp — 在右下角绘制技能冷却状态
// 布局（百分比）：
//   右上角：技能图标区域
//   圆形/条形冷却指示器
//   技能名称
//   就绪/冷却中/激活中 文字状态
```

示例绘制逻辑：
```
if (m_pSkillActive && *m_pSkillActive) {
    // 金色闪烁边框 → 技能激活中
    painter.setPen(QColor(255, 215, 0, 200));
    // 绘制"技能激活"文字
} else if (m_pSkillReady && *m_pSkillReady) {
    // 绿色 → 技能就绪
    painter.setPen(QColor(0, 255, 100));
    painter.drawText(..., QStringLiteral("[SPACE] 技能就绪"));
} else if (m_pSkillCooldown) {
    // 灰色 → 冷却中，显示百分比
    float pct = *m_pSkillCooldown;
    painter.setPen(QColor(150, 150, 150));
    painter.drawText(..., QString("技能冷却 %1%").arg(static_cast<int>(pct * 100)));
}
```

### 技能特效（GameScene）

在 `GameScene::drawForeground` 中，当技能激活时绘制视觉反馈：

- **雷暴领域**（ThunderStrike）：全屏白色闪烁（短暂 overlay）
- **极寒护盾**（FrostShield）：玩家周围蓝色光圈
- **铁壁阵**（IronWall）：玩家周围金色光环
- **火焰风暴**（FlameStorm）：玩家周围红色粒子效果（可选）

（如果时间紧张，可先简单实现光圈效果，使用 `drawEllipse` 绘制半透明圆环。）

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### QRC 资源确认

`resources/resources.qrc` 已有 5 架战机图片 alias：

```xml
<file alias="thunderShip">images/MyAircraft/Firepower.png</file>
<file alias="flameShip">images/MyAircraft/Flame.png</file>
<file alias="frostShip">images/MyAircraft/Frost.png</file>
<file alias="phantomShip">images/MyAircraft/Speed.png</file>
<file alias="fortressShip">images/MyAircraft/Defense.png</file>
```

✅ **QRC 已就绪，无需修改**。

### ⚠️ 注意：旧注释代码的 alias 名称与 QRC 不匹配

AppAgent 中有 4 行被注释掉的旧代码使用了错误的 alias 名：

```cpp
// ❌ 错误：QRC 中无此 alias
// m_spriteVM->setAircraftPixmap(AircraftType::Flame, assets.getImage("aircraftFlame"));
// m_spriteVM->setAircraftPixmap(AircraftType::Frost, assets.getImage("aircraftFrost"));
// m_spriteVM->setAircraftPixmap(AircraftType::Phantom, assets.getImage("aircraftSpeed"));
// m_spriteVM->setAircraftPixmap(AircraftType::Fortress, assets.getImage("aircraftDefense"));
```

正确的加载方式（已在 App 章节给出）：
```cpp
// ✅ 正确：与 QRC alias 一致
m_spriteVM->setAircraftPixmap(AircraftType::Flame,    assets.getImage("flameShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Frost,    assets.getImage("frostShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Phantom,  assets.getImage("phantomShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Fortress, assets.getImage("fortressShip"));
```

**Resource Agent 只需确认 QRC aliases 正确即可，无需改动代码。**

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### 需要修改的文件 ❌

| 文件 | 变更 |
|---|---|
| `src/app/AppAgent.cpp` | 加载 5 架战机图片；注入 selectAircraftCommand/useSkillCommand；建立桥接变量 |

#### AppAgent::init() 新增逻辑

```cpp
// ── 加载 5 架战机图片（取消注释，修正 QRC alias 名称） ──
m_spriteVM->setAircraftPixmap(AircraftType::Thunder,  assets.getImage("thunderShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Flame,    assets.getImage("flameShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Frost,    assets.getImage("frostShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Phantom,  assets.getImage("phantomShip"));
m_spriteVM->setAircraftPixmap(AircraftType::Fortress, assets.getImage("fortressShip"));

// ── 注入所有 5 架战机图片到 GameView（用于 AircraftSelectScreen 预览） ──
// 可选：如果 AircraftSelectScreen 需要显示战机小图
// 当前 AircraftSelectScreen 是纯文字，不需要图片注入
// 但在游戏内切换图片时需要 SpiritVM → GameView 传递
// 现有代码已有 m_gameView->setPlayerPixmap(m_spriteVM->getAircraftPixmap(AircraftType::Thunder));
// 改为在 selectAircraft 命令执行时动态更新

// ── 新增命令绑定 ──
m_gameView->setSelectAircraftCommand(m_mapVM->getSelectAircraftCommand());
m_gameView->setUseSkillCommand(m_mapVM->getUseSkillCommand());

// ── 新增桥接变量 ──
// 在 m_bridge 区域添加：
m_bridgeSkillCooldown = 0.0f;
m_bridgeSkillReady    = true;
m_bridgeSkillActive   = false;
m_bridgeAircraftName  = m_mapVM->getAircraftName();

// 注入指针到 GameView/Hud
m_gameView->setSkillCooldownPtr(&m_bridgeSkillCooldown);
m_gameView->setSkillReadyPtr(&m_bridgeSkillReady);
// ...

// 在 onViewModelChanged 中更新：
case PROP_ID_SKILL_COOLDOWN:
    m_bridgeSkillCooldown = m_mapVM->getSkillCooldownPercent();
    m_bridgeSkillReady    = m_mapVM->isSkillReady();
    m_bridgeSkillActive   = m_mapVM->isSkillActive();
    break;
```

#### GameView — 新增 setter 方法

```cpp
// GameView.hpp — 新增
void setSkillCooldownPtr(const float* p) noexcept;
void setSkillReadyPtr(const bool* p) noexcept;
void setSkillActivePtr(const bool* p) noexcept;
void setAircraftNamePtr(const char* p) noexcept;
// 转发给 HudOverlay/GameScene
```

#### AppAgent 桥接变量新增

```cpp
// AppAgent.hpp — 新增
float m_bridgeSkillCooldown = 0.0f;
bool  m_bridgeSkillReady    = true;
bool  m_bridgeSkillActive   = false;
const char* m_bridgeAircraftName = "雷霆号";
```

---

## 流程变更总结

```
当前流程：
  Start → ModeSelect → LevelSelect → Game → GameOver

迭代 5 新流程：
  Start → AircraftSelect → ModeSelect → LevelSelect → Game → GameOver
            ↑ 新增

更新 GameState 枚举（Types.hpp）：
  enum class GameState {
      Menu,            // 开始菜单
      AircraftSelect,  // ← 新增：战机选择
      ModeSelect,      // 模式选择
      LevelSelect,     // 关卡选择
      Playing,         // 游戏中
      Paused,          // 暂停
      GameOver         // 游戏结束
  };
```

---

## 开发顺序

```
① Common (Types.hpp 新增 AircraftSelect 枚举)  — 2 分钟
     ↓
③ View (AircraftSelectScreen 新建 + GameView 页面重排 + HUD技能显示)  ← 主力
     ↓
⑤ App (加载5战机图片 + 新命令绑定 + 桥接变量)  — 15 分钟
     ↓
② ViewModel (确认已有命令/getter，无新增)  — 5 分钟
     ↓
⑥ Test (可选：测试技能系统与战机选择)  — 10 分钟
```
