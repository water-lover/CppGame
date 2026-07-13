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
    QJsonArray arr = obj.value("upgradeLevels").toArray();
    for (int i = 0; i < 5 && i < arr.size(); ++i)
        data.levelsPacked[i] = arr[i].toInt(0);
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
