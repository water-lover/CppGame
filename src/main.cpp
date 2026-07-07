#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QFileInfo>
#include <QtQuick>

#include "Model/GameModel.hpp"
#include "ViewModel/GameViewModel.hpp"

int main(int argc, char *argv[])
{
    // ═══════════════════════════════════════════════════════════════════
    //  设置 Qt 平台插件路径（必须在 QGuiApplication 创建之前！）
    //  不能用 QCoreApplication::applicationDirPath() 因为 App 还未创建，
    //  改用 argv[0] 获取 exe 所在目录。
    // ═══════════════════════════════════════════════════════════════════
    if (argc > 0 && argv[0]) {
        QString exePath = QString::fromLocal8Bit(argv[0]);
        QFileInfo fi(exePath);
        QString pluginDir = fi.absolutePath() + QStringLiteral("/plugins");
        if (QDir(pluginDir).exists()) {
            qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", pluginDir.toLocal8Bit());
            qputenv("QT_PLUGIN_PATH", pluginDir.toLocal8Bit());
        }
    }

    // ── Qt 应用程序 ────────────────────────────────────────────────────
    QGuiApplication app(argc, argv);
    app.setApplicationName("CppGame");
    app.setApplicationVersion("0.1.0");

    // ── MVVM 初始化 ────────────────────────────────────────────────────
    auto model      = std::make_shared<GameModel>();
    auto viewModel  = new GameViewModel(model);   // QObject，Qt 管理生命周期

    // ── QML 引擎 ───────────────────────────────────────────────────────
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("viewModel_", viewModel);

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
