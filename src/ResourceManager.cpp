#include "ResourceManager.hpp"
#include <iostream>

ResourceManager& ResourceManager::instance() {
    static ResourceManager inst;
    return inst;
}

// ── 纹理 ──────────────────────────────────────────────────────────────────

sf::Texture& ResourceManager::loadTexture(const std::string& path) {
    auto it = textures_.find(path);
    if (it != textures_.end())
        return it->second;

    sf::Texture tex;
    if (!tex.loadFromFile(path)) {
        std::cerr << "[ResourceManager] Failed to load texture: " << path << std::endl;
        throw std::runtime_error("Cannot load texture: " + path);
    }
    auto [newIt, _] = textures_.emplace(path, std::move(tex));
    std::cout << "[ResourceManager] Loaded texture: " << path << std::endl;
    return newIt->second;
}

bool ResourceManager::hasTexture(const std::string& path) const {
    return textures_.find(path) != textures_.end();
}

// ── 字体 ──────────────────────────────────────────────────────────────────

sf::Font& ResourceManager::loadFont(const std::string& path) {
    auto it = fonts_.find(path);
    if (it != fonts_.end())
        return it->second;

    sf::Font font;
    if (!font.openFromFile(path)) {
        std::cerr << "[ResourceManager] Failed to load font: " << path << std::endl;
        throw std::runtime_error("Cannot load font: " + path);
    }
    auto [newIt, _] = fonts_.emplace(path, std::move(font));
    std::cout << "[ResourceManager] Loaded font: " << path << std::endl;
    return newIt->second;
}

bool ResourceManager::hasFont(const std::string& path) const {
    return fonts_.find(path) != fonts_.end();
}

// ── 音效 ──────────────────────────────────────────────────────────────────

sf::SoundBuffer& ResourceManager::loadSoundBuffer(const std::string& path) {
    auto it = soundBuffers_.find(path);
    if (it != soundBuffers_.end())
        return it->second;

    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path)) {
        std::cerr << "[ResourceManager] Failed to load sound: " << path << std::endl;
        throw std::runtime_error("Cannot load sound: " + path);
    }
    auto [newIt, _] = soundBuffers_.emplace(path, std::move(buffer));
    std::cout << "[ResourceManager] Loaded sound: " << path << std::endl;
    return newIt->second;
}

bool ResourceManager::hasSoundBuffer(const std::string& path) const {
    return soundBuffers_.find(path) != soundBuffers_.end();
}

// ── 清空 ──────────────────────────────────────────────────────────────────

void ResourceManager::clearAll() {
    textures_.clear();
    fonts_.clear();
    soundBuffers_.clear();
    std::cout << "[ResourceManager] All resources cleared." << std::endl;
}

void ResourceManager::clearUnused() {
    // 简单实现：清空所有单引用资源
    // 后续可用 shared_ptr + weak_ptr 做精细管理
}
