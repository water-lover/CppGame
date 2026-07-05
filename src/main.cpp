#include <SFML/Graphics.hpp>
#include <iostream>

#include "Model/GameModel.hpp"
#include "ViewModel/GameViewModel.hpp"
#include "View/GameView.hpp"

int main()
{
    // ── 创建 SFML 窗口 ────────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode({800, 600}), "CppGame");
    window.setFramerateLimit(60);

    // ── MVVM 初始化 ────────────────────────────────────────────────────
    auto model      = std::make_shared<GameModel>();
    auto viewModel  = std::make_shared<GameViewModel>(model);
    auto view       = std::make_shared<GameView>(window);

    viewModel->initialize();
    view->bindViewModel(viewModel);

    // ── 时钟 ────────────────────────────────────────────────────────────
    sf::Clock clock;

    // ── 主循环 ──────────────────────────────────────────────────────────
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // 1. 处理事件
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // 2. 更新 ViewModel（ViewModel 内部更新 Model）
        viewModel->update(deltaTime);

        // 3. 渲染
        window.clear(sf::Color::Black);

        view->render();

        window.display();
    }

    return 0;
}
