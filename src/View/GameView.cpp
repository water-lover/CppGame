#include "View/GameView.hpp"
#include "ViewModel/IViewModel.hpp"
#include <iostream>

GameView::GameView(sf::RenderWindow& window)
    : window_(window) {}

void GameView::bindViewModel(std::shared_ptr<IViewModel> viewModel) {
    viewModel_ = std::move(viewModel);
    viewModel_->addPropertyListener(
        [this](const std::string& prop) { onPropertyChanged(prop); });
}

void GameView::render() {
    // ── 由 ViewModel 驱动渲染逻辑 ──
    // 后续在此绘制游戏元素
}

void GameView::handleInput() {
    // ── 输入事件在 main.cpp 的事件循环中处理 ──
    // 后续可将复杂输入逻辑移入此处
}

void GameView::onPropertyChanged(const std::string& property) {
    // 属性变化时更新界面元素
    // std::cout << "Property changed: " << property << std::endl;
}
