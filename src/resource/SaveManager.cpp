#include "resource/SaveManager.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>

SaveManager::SaveManager() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    filePath_ = dir + QStringLiteral("/save.json");
}

// ── 内部辅助：读取完整 JSON ──────────────────────────────────────────

static QJsonObject loadObject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    return doc.isObject() ? doc.object() : QJsonObject();
}

static void saveObject(const QString& path, const QJsonObject& obj) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
}

// ── 最高分 ────────────────────────────────────────────────────────────

int SaveManager::loadHighScore() {
    return loadObject(filePath_).value("highScore").toInt(0);
}

void SaveManager::saveHighScore(int score) {
    QJsonObject obj = loadObject(filePath_);
    obj["highScore"] = score;
    saveObject(filePath_, obj);
}

// ── 闯关进度（迭代 3） ────────────────────────────────────────────────

int SaveManager::loadCampaignProgress() {
    return loadObject(filePath_).value("campaignLevel").toInt(1);
}

void SaveManager::saveCampaignProgress(int level) {
    QJsonObject obj = loadObject(filePath_);
    obj["campaignLevel"] = qBound(1, level, 7);
    saveObject(filePath_, obj);
}

// ── 升级数据（迭代 6） ────────────────────────────────────────────────

int SaveManager::packLevels(int fire, int lives, int speed, int cooldown) {
    return (fire    & 0xF)       |
           ((lives   & 0xF) << 4)  |
           ((speed   & 0xF) << 8)  |
           ((cooldown & 0xF) << 12);
}

SaveManager::UpgradeData SaveManager::loadUpgradeData() {
    QJsonObject obj = loadObject(filePath_);
    UpgradeData data;
    data.starCores    = obj.value("starCores").toInt(0);
    data.levelsPacked = obj.value("upgradeLevels").toInt(0);
    return data;
}

void SaveManager::saveUpgradeData(const UpgradeData& data) {
    QJsonObject obj = loadObject(filePath_);
    obj["starCores"]     = data.starCores;
    obj["upgradeLevels"] = data.levelsPacked;
    saveObject(filePath_, obj);
}
