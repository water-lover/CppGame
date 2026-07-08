#ifndef SAVEMANAGER_HPP
#define SAVEMANAGER_HPP

#include <QString>

class SaveManager {
public:
    SaveManager();

    int  loadHighScore();
    void saveHighScore(int score);

private:
    QString filePath_;
};

#endif // SAVEMANAGER_HPP
