#include "ViewModel/GameViewModel.hpp"

GameViewModel::GameViewModel(std::shared_ptr<GameModel> model, QObject* parent)
    : QObject(parent), model_(std::move(model)) {}

int   GameViewModel::score()      const { return model_ ? model_->getScore()     : 0;    }
bool  GameViewModel::running()    const { return model_ ? model_->isRunning()    : false; }
float GameViewModel::targetX()    const { return model_ ? model_->getTargetX()   : 0.5f;  }
float GameViewModel::targetY()    const { return model_ ? model_->getTargetY()   : 0.5f;  }
float GameViewModel::targetSize() const { return model_ ? model_->getTargetSize(): 0.08f; }
float GameViewModel::timeLeft()   const { return model_ ? model_->getTimeLeft()  : 0.0f;  }
int   GameViewModel::level()      const { return model_ ? model_->getLevel()     : 1;     }
int   GameViewModel::totalTime()  const { return 30; }

void GameViewModel::startGame() {
    if (!model_) return;
    model_->reset();
    emit scoreChanged(model_->getScore());
    emit runningChanged(true);
    emit targetChanged();
    emit timeChanged(model_->getTimeLeft());
    emit levelChanged(model_->getLevel());
    emit gameStarted();
}

void GameViewModel::quitGame() {
    if (!model_) return;
    model_->setRunning(false);
    emit runningChanged(false);
    emit gameOver();
}

void GameViewModel::clickTarget(float mx, float my) {
    if (!model_ || !model_->isRunning()) return;
    bool hit = model_->hitTarget(mx, my);
    if (hit) {
        emit scoreChanged(model_->getScore());
        emit targetChanged();
        emit levelChanged(model_->getLevel());
    }
}

void GameViewModel::tick(float deltaTime) {
    if (!model_) return;
    bool wasRunning = model_->isRunning();
    model_->update(deltaTime);
    bool nowRunning = model_->isRunning();

    emit targetChanged();
    emit timeChanged(model_->getTimeLeft());

    if (wasRunning && !nowRunning) {
        emit runningChanged(false);
        emit gameOver();
    }
}
