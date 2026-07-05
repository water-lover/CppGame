#ifndef IMODEL_HPP
#define IMODEL_HPP

#include <string>

// ── IModel ─────────────────────────────────────────────────────────────────
/// @brief 纯虚基类 — 代表游戏数据/状态模型。
///        ViewModel 通过 IModel 接口读写数据，View 不直接接触 Model。
struct IModel {
    virtual ~IModel() = default;

    /// 初始化 / 重置模型到初始状态
    virtual void reset() = 0;

    /// 每帧更新模型状态（deltaTime 单位为秒）
    virtual void update(float deltaTime) = 0;
};

#endif // IMODEL_HPP
