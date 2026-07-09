# 迭代 2 — 画面提升 + 屏幕适应：Agent 指令

> **整体目标**：高画质素材 + 窗口自适应 + 模式选择 + 暂停功能
>
> **框架依据**：`docs/planning/DESIGN_PLAN.md` 和 `docs/planning/RULES.md`

---

## ① Common Agent

**目录**：`src/common/` + `include/common/`

### 需要修改的文件

| 文件 | 变更 |
|---|---|
| `include/common/Constants.hpp` | 新增 `FULLSCREEN_TOGGLE_KEY = Qt::Key_F11`，`MIN_WINDOW_WIDTH = 600`，`MIN_WINDOW_HEIGHT = 450` |
| `include/common/Types.hpp` | `GameState` 增加 `ModeSelect, Paused`；`ActorType` 增加 `PowerUpHp, PowerUpFire, PowerUpShield` |

---

## ③ View Agent（主力）

**目录**：`src/view/` + `include/view/`

### 核心变更 1：屏幕适应性

```cpp
// GameView.hpp — 去掉 setFixedSize，改为可缩放
class GameView : public QWidget {
    // ...
protected:
    void resizeEvent(QResizeEvent* e) override {
        QWidget::resizeEvent(e);
        if (m_graphicsView)
            m_graphicsView->fitInView(0, 0, 800, 600, Qt::KeepAspectRatio);
    }
    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_F11) {
            if (isFullScreen()) showNormal();
            else showFullScreen();
        }
        // ... 保留原有按键 ...
    }
};
```

### 核心变更 2：HUD 百分比布局

```cpp
// HudOverlay.cpp — 改用百分比定位
void HudOverlay::paintEvent(QPaintEvent*) {
    float w = width();
    float h = height();

    // 分数 — 左上角 2%, 2%
    painter->drawText(QRectF(w * 0.02, h * 0.02, w * 0.3, h * 0.05), ...);
    // 生命 — 右上角
    painter->drawText(QRectF(w * 0.68, h * 0.02, w * 0.3, h * 0.05), ...);
    // 波次 — 中上方
    painter->drawText(QRectF(w * 0.35, h * 0.02, w * 0.3, h * 0.05), ...);
}
```

### 需要创建/修改的文件

| 文件 | 变更 |
|---|---|
| `src/view/GameView.cpp` | 去掉 `setFixedSize`；实现 `resizeEvent` + `fitInView`；F11 全屏；窗口拉伸 |
| `src/view/HudOverlay.cpp` | 改为百分比定位（分数左上、生命右上、波次居中上方）；支持 `setWave(int)` |
| `src/view/GameScene.cpp` | 高分辨率渲染：确保 `SmoothPixmapTransform` 开启 |
| `src/view/ModeSelectScreen.hpp/cpp` | **新建** — 模式选择界面：标题 + "闯关模式" / "无尽模式" 两个大按钮，emit `modeSelected(int)` |
| `src/view/PauseOverlay.hpp/cpp` | **新建** — 暂停覆盖层：半透明遮罩 + "继续游戏"按钮，emit `resumeClicked()` |

### 界面切换更新

```
GameView 的 QStackedWidget：
  index 0: StartScreen
  index 1: ModeSelectScreen     ← 新增
  index 2: GamePage（游戏场景 + HUD）
  index 3: GameOverScreen
  index 4: PauseOverlay         ← 新增

// GameView 新增 setter
void setSelectModeCommand(std::function<void(int)>&& cmd);
void setPauseCommand(std::function<void()>&& cmd);
```

---

## ④ Resource Agent

**目录**：`src/resource/` + `include/resource/`

### QRC 资源新增

`resources/resources.qrc` 新增高分辨率素材的 alias：

```xml
<file alias="enemyMedium">images/PNG/Enemies/enemyBlack1.png</file>
<file alias="enemyLarge">images/PNG/Enemies/enemyBlack5.png</file>
<file alias="bossShip">images/PNG/Sprites/Ships/spaceShips_009.png</file>
<file alias="bossRocket">images/PNG/Sprites/Rockets/spaceRockets_001.png</file>
<file alias="enemyBullet">images/PNG/Lasers/laserRed01.png</file>
<file alias="powerUpHp">images/PNG/Power-ups/pill_red.png</file>
<file alias="powerUpFire">images/PNG/Power-ups/powerupBlue_bolt.png</file>
<file alias="powerUpShield">images/PNG/Power-ups/shield_gold.png</file>
```

---

## ⑤ App Agent

**目录**：`src/app/` + `include/app/`

### 需要修改的文件

`src/app/AppAgent.cpp`：

```cpp
void AppAgent::init() {
    // ... 迭代1原有逻辑 ...

    // SpiritVM 注入新图片（供后续迭代使用，但 App 先注册）
    m_spriteVM->setEnemyMediumPixmap(assets.getImage("enemyMedium"));
    m_spriteVM->setEnemyLargePixmap(assets.getImage("enemyLarge"));
    m_spriteVM->setBossPixmap(assets.getImage("bossShip"));
    m_spriteVM->setEnemyBulletPixmap(assets.getImage("enemyBullet"));
    m_spriteVM->setPowerUpHpPixmap(assets.getImage("powerUpHp"));
    m_spriteVM->setPowerUpFirePixmap(assets.getImage("powerUpFire"));
    m_spriteVM->setPowerUpShieldPixmap(assets.getImage("powerUpShield"));

    // 新增命令绑定
    m_gameView->setSelectModeCommand(m_mapVM->getSelectModeCommand());
    m_gameView->setPauseCommand(m_mapVM->getPauseCommand());
}
```

---

## 开发顺序

```
① Common (枚举/常量)
     ↓
③ View (屏幕适应 → HUD百分比 → ModeSelectScreen → PauseOverlay) ← 主力
     ↓
④ Resource (QRC新增)  ← 可与 View 并行
     ↓
⑤ App (组装)
```
