#include "app/AppAgent.hpp"
#include "logic/GameModel.hpp"
#include "logic/GameViewModel.hpp"
#include "resource/SaveManager.hpp"
#include "common/Logger.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>

AppAgent::AppAgent() = default;

AppAgent::~AppAgent() {
    delete viewModel_;
    delete engine_;
}

void AppAgent::init() {
    log("AppAgent", "Initializing Thunder Fighter...");

    // ── 创建 Model ────────────────────────────────────────────────────
    model_ = std::make_shared<GameModel>();

    // ── 创建 ViewModel ────────────────────────────────────────────────
    viewModel_ = new GameViewModel(model_);

    // ── 加载最高分 ────────────────────────────────────────────────────
    SaveManager saveMgr;
    int highScore = saveMgr.loadHighScore();
    model_->getScoreMgr().setHighScore(highScore);
    log("AppAgent", "Loaded high score: " + std::to_string(highScore));

    // ── 创建 QML 引擎 ────────────────────────────────────────────────
    engine_ = new QQmlApplicationEngine();
    engine_->rootContext()->setContextProperty(QStringLiteral("vm"), viewModel_);

    // ── 加载 QML ──────────────────────────────────────────────────────
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    engine_->load(url);

    log("AppAgent", "Initialization complete");
}

int AppAgent::run() {
    if (engine_->rootObjects().isEmpty()) {
        log("AppAgent", "Error: No root QML objects loaded");
        return -1;
    }

    return qGuiApp->exec();
}
