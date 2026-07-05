#include "ViewModel/GameViewModel.hpp"

GameViewModel::GameViewModel(std::shared_ptr<GameModel> model)
    : model_(std::move(model)) {}

void GameViewModel::initialize() {
    model_->reset();
    notifyPropertyChanged("score");
    notifyPropertyChanged("running");
}

void GameViewModel::update(float deltaTime) {
    model_->update(deltaTime);
    // 将 Model 数据同步到可观察属性（后续可扩展）
}

void GameViewModel::addPropertyListener(const PropertyCallback& cb) {
    listeners_.push_back(cb);
}

void GameViewModel::notifyPropertyChanged(const std::string& propertyName) {
    for (auto& cb : listeners_) {
        cb(propertyName);
    }
}

int  GameViewModel::getScore()  const { return model_->getScore(); }
bool GameViewModel::isRunning() const { return model_->isRunning(); }

void GameViewModel::startGame() {
    model_->reset();
    notifyPropertyChanged("score");
    notifyPropertyChanged("running");
}

void GameViewModel::quitGame() {
    model_->setRunning(false);
    notifyPropertyChanged("running");
}
