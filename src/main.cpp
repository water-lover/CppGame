#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQuick>

#include "Model/GameModel.hpp"
#include "ViewModel/GameViewModel.hpp"

int main(int argc, char *argv[])
{
    // ── Qt 应用程序（无窗口系统依赖） ────────────────────────────────
    QGuiApplication app(argc, argv);
    app.setApplicationName("CppGame");
    app.setApplicationVersion("0.1.0");

    // ── MVVM 初始化 ────────────────────────────────────────────────────
    auto model      = std::make_shared<GameModel>();
    auto viewModel  = new GameViewModel(model);   // QObject，Qt 管理生命周期

    // ── QML 引擎 ───────────────────────────────────────────────────────
    QQmlApplicationEngine engine;

    // 将 ViewModel 注入 QML 上下文（名字为 "viewModel_"）
    engine.rootContext()->setContextProperty("viewModel_", viewModel);

    // 加载主 QML 文件（先尝试 qrc，再尝试文件系统）
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
