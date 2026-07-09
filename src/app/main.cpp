#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include "app/AppAgent.hpp"
#include "common/Logger.hpp"

int main(int argc, char *argv[])
{
    // ── Qt 应用程序 ────────────────────────────────────────────────────
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ThunderFighter"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    // ── 启动游戏（App 层只做组装，不写逻辑/渲染） ────────────────────
    AppAgent gameApp;
    gameApp.init();

    // 显示窗口并进入事件循环
    return gameApp.run();
}
