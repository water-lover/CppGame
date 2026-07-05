#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>
#include <memory>

// ── ResourceManager ────────────────────────────────────────────────────────
/// @brief 游戏资源管理器 — 加载、缓存、回收纹理/字体/音效。
///        单例模式，全局共享一份资源缓存。
class ResourceManager {
public:
    static ResourceManager& instance();

    // ── 纹理 ────────────────────────────────────────────────────────────
    sf::Texture& loadTexture(const std::string& path);
    bool hasTexture(const std::string& path) const;

    // ── 字体 ────────────────────────────────────────────────────────────
    sf::Font& loadFont(const std::string& path);
    bool hasFont(const std::string& path) const;

    // ── 音效 ────────────────────────────────────────────────────────────
    sf::SoundBuffer& loadSoundBuffer(const std::string& path);
    bool hasSoundBuffer(const std::string& path) const;

    // ── 清空缓存（切换场景时调用） ────────────────────────────────────
    void clearAll();
    void clearUnused();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::unordered_map<std::string, sf::Texture>     textures_;
    std::unordered_map<std::string, sf::Font>        fonts_;
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffers_;
};

#endif // RESOURCEMANAGER_HPP
