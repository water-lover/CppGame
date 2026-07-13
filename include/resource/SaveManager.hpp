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

private:
    QString filePath_;
};

#endif // SAVEMANAGER_HPP
