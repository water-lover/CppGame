#ifndef SAVEMANAGER_HPP
#define SAVEMANAGER_HPP

#include <QString>
#include <cstdint>

/// 存档管理器 — 唯一负责读写磁盘 JSON 存档的类
///
/// 存储数据：
///   - highScore:       最高分（迭代 1）
///   - campaignLevel:   闯关模式进度（迭代 3）
///   - starCores:       星核碎片数量（迭代 6）
///   - upgradeLevels:   4 项升级等级打包（迭代 6）
class SaveManager {
public:
    SaveManager();

    // ── 最高分 ────────────────────────────────────────
    int  loadHighScore();
    void saveHighScore(int score);

    // ── 闯关进度（迭代 3） ─────────────────────────────
    int  loadCampaignProgress();
    void saveCampaignProgress(int level);

    // ── 升级数据（迭代 6） ─────────────────────────────
    /// 升级数据包
    struct UpgradeData {
        int starCores   = 0;    // 星核碎片数量
        int levelsPacked = 0;   // 4 项等级打包（每项 4bit）
    };

    UpgradeData loadUpgradeData();
    void saveUpgradeData(const UpgradeData& data);

    /// 重置所有存档数据（最高分/升级数据/关卡进度）
    void resetAllData();

    /// 打包：将 4 项等级压缩为 16bit int
    static int packLevels(int fire, int lives, int speed, int cooldown);

private:
    QString filePath_;
};

#endif // SAVEMANAGER_HPP
