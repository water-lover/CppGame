#ifndef SAVEMANAGER_HPP
#define SAVEMANAGER_HPP

#include <QString>

/// 存档管理器 — 唯一负责读写磁盘 JSON 存档的类
///
/// 存储数据：
///   - highScore:    最高分（迭代 1）
///   - campaignLevel: 闯关模式进度（迭代 3）
class SaveManager {
public:
    SaveManager();

    // ── 最高分 ────────────────────────────────────────
    int  loadHighScore();
    void saveHighScore(int score);

    // ── 闯关进度（迭代 3） ─────────────────────────────
    int  loadCampaignProgress();          // 返回 1~7，默认 1
    void saveCampaignProgress(int level); // 保存当前通关进度

private:
    QString filePath_;
};

#endif // SAVEMANAGER_HPP
