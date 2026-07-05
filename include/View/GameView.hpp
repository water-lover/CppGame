#ifndef GAMEVIEW_HPP
#define GAMEVIEW_HPP

#include "View/IView.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class IViewModel;

// ── GameView ───────────────────────────────────────────────────────────────
/// @brief SFML 游戏视图 — 渲染画面、处理输入、绑定 ViewModel。
class GameView : public IView {
public:
    explicit GameView(sf::RenderWindow& window);
    ~GameView() override = default;

    void bindViewModel(std::shared_ptr<IViewModel> viewModel) override;
    void render() override;
    void handleInput() override;

private:
    sf::RenderWindow& window_;
    std::shared_ptr<IViewModel> viewModel_;

    // 当 ViewModel 属性变化时更新界面
    void onPropertyChanged(const std::string& property);
};

#endif // GAMEVIEW_HPP
