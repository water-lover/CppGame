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

int SaveManager::loadHighScore() {
    QFile file(filePath_);
    if (!file.open(QIODevice::ReadOnly))
        return 0;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isObject()) {
        return doc.object().value("highScore").toInt(0);
    }
    return 0;
}

void SaveManager::saveHighScore(int score) {
    QJsonObject obj;
    obj["highScore"] = score;

    QFile file(filePath_);
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
}
