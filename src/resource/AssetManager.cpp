#include "resource/AssetManager.hpp"
#include <QDebug>

AssetManager& AssetManager::instance() {
    static AssetManager inst;
    return inst;
}

const QPixmap* AssetManager::getImage(const QString& key) {
    // ① 检查缓存
    auto it = m_cache.find(key);
    if (it != m_cache.end())
        return &it->second;

    // ② 从 QRC 资源系统加载
    QString path = QStringLiteral(":/images/%1").arg(key);
    QPixmap pix(path);
    if (pix.isNull()) {
        qWarning() << "AssetManager: failed to load" << path;
        return nullptr;
    }

    // ③ 存入缓存，返回 const 指针
    auto result = m_cache.emplace(key, std::move(pix));
    return &result.first->second;
}
