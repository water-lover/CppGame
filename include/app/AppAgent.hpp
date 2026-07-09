#ifndef APPAGENT_HPP
#define APPAGENT_HPP

#include <cstdint>

#include "common/PropertyIds.hpp"
#include "common/Types.hpp"

class GameView;
class GameMapVM;
class SpiritVM;

/// 应用生命周期层 — 唯一组装者
///
/// 严格 MVVM 职责：
///   - 创建所有 Agent 实例
///   - ① 属性绑定 — 将 ViewModel 的数据指针注入 View
///   - ② 命令绑定 — 将 ViewModel 的 std::function 命令注入 View
///   - ③ 事件绑定 — 连接 ViewModel 的 signal 到 View 的 slot
///   - 只做组装，不写游戏逻辑或渲染代码
///
/// 帧循环由 View 层的 GameView 自行管理（内部 QTimer），App 不干涉。
class AppAgent {
public:
    AppAgent();
    ~AppAgent();

    /// 初始化所有 Agent 并建立三绑定连接
    void init();

    /// 进入 Qt 事件循环
    int  run();

private:
    /// 收到 ViewModel 的 propertyChanged 信号后，更新桥接变量
    void onViewModelChanged(uint32_t propertyId);

    // ── 所有 Agent 实例 ──────────────────────────────────────────
    GameMapVM*        m_mapVM     = nullptr;
    SpiritVM*   m_spriteVM  = nullptr;
    GameView*         m_gameView  = nullptr;

    // ── 数据桥接（const T* 属性绑定需要的稳定内存地址） ─────────
    int       m_bridgeScore     = 0;
    int       m_bridgeLives     = 3;
    int       m_bridgeHighScore = 0;
    GameState m_bridgeState     = GameState::Menu;
};

#endif // APPAGENT_HPP
