#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include <QPixmap>
#include <QString>
#include <map>

/// 资源管理器 — 唯一负责从 QRC 加载 PNG 图片的 Agent
///
/// MVVM 职责：
///   - 所有文件 I/O 必须通过此类（View 和 ViewModel 不能直接读写文件）
///   - 返回 const QPixmap* 指针，对齐 MVVM 的 const T* 属性绑定模式
///   - 内部缓存已加载的图片，避免重复读盘
class AssetManager {
public:
    static AssetManager& instance();

    /// 获取图片（const QPixmap* 只读指针，MVVM 属性绑定）
    const QPixmap* getImage(const QString& key);

private:
    AssetManager() = default;

    std::map<QString, QPixmap> m_cache;
};

#endif // ASSETMANAGER_HPP
