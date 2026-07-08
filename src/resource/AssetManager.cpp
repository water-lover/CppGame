#include "resource/AssetManager.hpp"
#include <QDir>
#include <QDebug>

AssetManager& AssetManager::instance() {
    static AssetManager inst;
    return inst;
}

QPixmap AssetManager::getImage(const QString& key) {
    auto it = cache_.find(key);
    if (it != cache_.end())
        return it->second;

    // 从 qrc 资源系统加载
    QString path = QStringLiteral(":/images/%1").arg(key);
    QPixmap pix(path);
    if (pix.isNull()) {
        qWarning() << "AssetManager: failed to load" << path;
        return {};
    }
    cache_[key] = pix;
    return pix;
}
