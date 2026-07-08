#ifndef APPAGENT_HPP
#define APPAGENT_HPP

#include <memory>

class GameModel;
class GameViewModel;
class QQmlApplicationEngine;

class AppAgent {
public:
    AppAgent();
    ~AppAgent();

    void init();
    int  run();

private:
    std::shared_ptr<GameModel>       model_;
    GameViewModel*                    viewModel_ = nullptr;
    QQmlApplicationEngine*            engine_    = nullptr;
};

#endif // APPAGENT_HPP
