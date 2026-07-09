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
