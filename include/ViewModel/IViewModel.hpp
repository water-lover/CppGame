#ifndef IVIEWMODEL_HPP
#define IVIEWMODEL_HPP

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

// ── IViewModel ─────────────────────────────────────────────────────────────
/// @brief ViewModel 基类 — 连接 Model 与 View 的桥梁。
///        View 绑定到 ViewModel 暴露的属性/命令，ViewModel 从 Model 拉取数据。
class IViewModel {
public:
    virtual ~IViewModel() = default;

    // ── 生命周期 ────────────────────────────────────────────────────────
    /// 初始化（绑定 Model、注册命令等）
    virtual void initialize() = 0;

    /// 每帧更新（从 Model 同步数据到可观察属性）
    virtual void update(float deltaTime) = 0;

    // ── 属性通知 ─────────────────────────────────────────────────────────
    /// 注册一个属性变更回调（View 调用此方法绑定）
    using PropertyCallback = std::function<void(const std::string&)>;
    virtual void addPropertyListener(const PropertyCallback& cb) = 0;

    /// 触发属性变更通知（派生类在属性变化时调用）
    virtual void notifyPropertyChanged(const std::string& propertyName) = 0;
};

#endif // IVIEWMODEL_HPP
