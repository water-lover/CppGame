#ifndef GAMEVIEWMODEL_HPP
#define GAMEVIEWMODEL_HPP

#include "ViewModel/IViewModel.hpp"
#include "Model/GameModel.hpp"
#include <memory>
#include <vector>

// ── GameViewModel ──────────────────────────────────────────────────────────
/// @brief 游戏 ViewModel — 将 GameModel 的数据转换为 View 可绑定的属性。
class GameViewModel : public IViewModel {
public:
    explicit GameViewModel(std::shared_ptr<GameModel> model);
    ~GameViewModel() override = default;

    void initialize() override;
    void update(float deltaTime) override;

    void addPropertyListener(const PropertyCallback& cb) override;
    void notifyPropertyChanged(const std::string& propertyName) override;

    // ── View 可绑定的只读属性 ──────────────────────────────────────────
    int  getScore()  const;
    bool isRunning() const;

    // ── View 可调用的命令 ──────────────────────────────────────────────
    void startGame();
    void quitGame();

private:
    std::shared_ptr<GameModel> model_;
    std::vector<PropertyCallback> listeners_;
};

#endif // GAMEVIEWMODEL_HPP
