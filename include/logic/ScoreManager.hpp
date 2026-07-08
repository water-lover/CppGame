#ifndef SCOREMANAGER_HPP
#define SCOREMANAGER_HPP

class ScoreManager {
public:
    ScoreManager();

    void reset();
    void addScore(int points);
    int  getScore()      const { return score_; }
    int  getHighScore()  const;

    // 由外部调用，通过 Resource Agent 持久化
    void setHighScore(int hs) { highScore_ = hs; }

private:
    int score_     = 0;
    int highScore_ = 0;
};

#endif // SCOREMANAGER_HPP
