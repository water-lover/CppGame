#ifndef IVIEW_HPP
#define IVIEW_HPP

#include <memory>

class IViewModel;

// ── IView ──────────────────────────────────────────────────────────────────
/// @brief View 基类 — 负责渲染 SFML 画面。
///        持有 ViewModel 引用，通过数据绑定驱动界面更新。
class IView {
public:
    virtual ~IView() = default;

    /// 绑定到 ViewModel
    virtual void bindViewModel(std::shared_ptr<IViewModel> viewModel) = 0;

    /// 每帧渲染
    virtual void render() = 0;

    /// 处理输入事件（由主循环转发）
    virtual void handleInput() = 0;
};

#endif // IVIEW_HPP
