#ifndef SPLASHSCREEN_HPP
#define SPLASHSCREEN_HPP

#include <QWidget>
#include <QLabel>

/// 加载过渡画面
class SplashScreen : public QWidget {
    Q_OBJECT

public:
    explicit SplashScreen(QWidget* parent = nullptr);

    void setMessage(const QString& msg);
    void setProgress(int percent);  // 0~100

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_message = QStringLiteral("加载中...");
    int m_progress = 0;
};

#endif // SPLASHSCREEN_HPP
