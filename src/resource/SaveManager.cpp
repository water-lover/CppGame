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

static QJsonObject loadObject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    return doc.isObject() ? doc.object() : QJsonObject();
}

static void saveObject(const QString& path, const QJsonObject& obj) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
}

int SaveManager::loadHighScore() {
    return loadObject(filePath_).value("highScore").toInt(0);
}

void SaveManager::saveHighScore(int score) {
    QJsonObject obj = loadObject(filePath_);
    obj["highScore"] = score;
    saveObject(filePath_, obj);
}

int SaveManager::loadCampaignProgress() {
    return loadObject(filePath_).value("campaignLevel").toInt(1);
}

void SaveManager::saveCampaignProgress(int level) {
    QJsonObject obj = loadObject(filePath_);
    obj["campaignLevel"] = qBound(1, level, 7);
    saveObject(filePath_, obj);
}

SaveManager::UpgradeData SaveManager::loadUpgradeData() {
    QJsonObject obj = loadObject(filePath_);
    UpgradeData data;
    data.starCores = obj.value("starCores").toInt(0);
    // 兼容旧格式：单 int 而非数组
    QJsonValue lvVal = obj.value("upgradeLevels");
    if (lvVal.isArray()) {
        QJsonArray arr = lvVal.toArray();
        for (int i = 0; i < 5 && i < arr.size(); ++i)
            data.levelsPacked[i] = arr[i].toInt(0);
    } else if (lvVal.isDouble()) {
        data.levelsPacked[0] = lvVal.toInt(0);
    }
    return data;
}

void SaveManager::saveUpgradeData(const UpgradeData& data) {
    QJsonObject obj = loadObject(filePath_);
    obj["starCores"] = data.starCores;
    QJsonArray arr;
    for (int i = 0; i < 5; ++i)
        arr.append(data.levelsPacked[i]);
    obj["upgradeLevels"] = arr;
    saveObject(filePath_, obj);
}

void SaveManager::resetAllData() {
    QJsonObject empty;
    saveObject(filePath_, empty);
}

// ── 分关卡最高分（兼容旧版 single highScore） ──────────────────

int SaveManager::loadCampaignHighScore(int level) {
    if (level < 1 || level > 7) return 0;
    QJsonObject obj = loadObject(filePath_);
    QJsonArray arr = obj.value("campaignScores").toArray();
    if (level <= arr.size()) return arr[level - 1].toInt(0);
    return obj.value("highScore").toInt(0);  // 降级：返回旧全局分
}

void SaveManager::saveCampaignHighScore(int level, int score) {
    if (level < 1 || level > 7) return;
    QJsonObject obj = loadObject(filePath_);
    QJsonArray arr = obj.value("campaignScores").toArray();
    while (arr.size() < 7) arr.append(0);
    // 不降级：仅当新分数更高时才覆盖
    if (score > arr[level - 1].toInt(0)) {
        arr[level - 1] = score;
        obj["campaignScores"] = arr;
        obj["highScore"] = score;  // 保留旧字段兼容
        saveObject(filePath_, obj);
    }
}

int SaveManager::loadEndlessHighScore() {
    return loadObject(filePath_).value("endlessScore").toInt(0);
}

void SaveManager::saveEndlessHighScore(int score) {
    QJsonObject obj = loadObject(filePath_);
    obj["endlessScore"] = score;
    saveObject(filePath_, obj);
}
