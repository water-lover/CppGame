#ifndef SAVEMANAGER_HPP
#define SAVEMANAGER_HPP

#include <QString>
#include <QJsonArray>

/// 存档管理器
class SaveManager {
public:
    SaveManager();

    int  loadHighScore();
    void saveHighScore(int score);

    int  loadCampaignProgress();
    void saveCampaignProgress(int level);

    struct UpgradeData {
        int starCores = 0;
        int levelsPacked[5] = {};   // 5 架战机各 16bit
    };

    UpgradeData loadUpgradeData();
    void saveUpgradeData(const UpgradeData& data);

    void resetAllData();

    // ── 分关卡 + 无尽最高分 ──────────────────────────────────
    int  loadCampaignHighScore(int level);            // level 1-7
    void saveCampaignHighScore(int level, int score);
    int  loadEndlessHighScore();
    void saveEndlessHighScore(int score);

private:
    QString filePath_;
};

#endif // SAVEMANAGER_HPP
