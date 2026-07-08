#include <QGuiApplication>
#include <QFileInfo>
#include <QDir>

#include "app/AppAgent.hpp"

int main(int argc, char *argv[])
{
    // ── 设置 Qt 平台插件路径 ──────────────────────────────────────────
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
    app.setApplicationName(QStringLiteral("ThunderFighter"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    // ── 启动游戏 ──────────────────────────────────────────────────────
    AppAgent gameApp;
    gameApp.init();
    return gameApp.run();
}
