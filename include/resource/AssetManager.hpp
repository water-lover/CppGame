#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include <QPixmap>
#include <QString>
#include <unordered_map>

class AssetManager {
public:
    static AssetManager& instance();

    QPixmap getImage(const QString& key);

private:
    AssetManager() = default;

    std::unordered_map<QString, QPixmap> cache_;
};

#endif // ASSETMANAGER_HPP
