// ── 自动化游戏演示 — 6 个阶段完整展示 ───────────────────────────────────────
// 阶段1: 第2关 + 雷霆号（第一架，完整流程含BOSS）
// 阶段2: 第3关 + 烈焰号（第二架，快速技能展示）
// 阶段3: 第4关 + 冰霜号（第三架，快速技能展示）
// 阶段4: 第5关 + 幻影号（第四架，快速技能展示）
// 阶段5: 无尽模式 + 堡垒号（第五架，技能+升级面板展示）

#include <catch2/catch_test_macros.hpp>
#include <QApplication>
#include <QtTest>
#include <QTimer>
#include <QPushButton>
#include <QStackedWidget>
#include <QDateTime>
#include <functional>

#include "view/GameView.hpp"
#include "view/StartScreen.hpp"
#include "view/ModeSelectScreen.hpp"
#include "viewmodel/GameMapVM.hpp"
#include "viewmodel/SpiritVM.hpp"

static void ensureApp() {
    static int argc = 1;
    static char arg0[] = "auto_demo";
    static char* argv[] = {arg0, nullptr};
    static QApplication app(argc, argv);
    Q_UNUSED(app);
}

// ── 坐标计算 ────────────────────────────────────────────────────────────────
// 窗口 800x600
// 关卡选择: btnW=144, btnH=84, gapX=32, gapY=15, startY=114
//   Row 0(y=114): L1(224), L2(400), L3(576)
//   Row 1(y=213): L4(224), L5(400), L6(576)
//   Row 2(y=312): L7(400)
static QPoint levelBtn(int id) {
    int r = (id-1)/3, c = (id-1)%3;
    return {152 + c*176 + 72, 114 + r*99 + 42};
}
// 战机选择: cardW=176, gapX=36, gapY=21, startY=90
//   Row 0(y=90, startX=100): T(188), F(400), Fr(612)
//   Row 1(y=309, startX=206): P(294), Fo(506)
static QPoint acCard(int idx) {
    if (idx < 3) return {100 + idx*212 + 88, 189};
    return {206 + (idx-3)*212 + 88, 408};
}

// ── 三绑定 ──────────────────────────────────────────────────────────────────
static void bindAll(GameView* v, GameMapVM* vm, SpiritVM* svm) {
    v->setMap(vm->getMap());
    v->setPlayerPixmap(svm->getAircraftPixmap(AircraftType::Thunder));
    v->setEnemySmallPixmap(svm->getEnemySmallPixmap());
    v->setEnemyMediumPixmap(svm->getEnemyMediumPixmap());
    v->setEnemyLargePixmap(svm->getEnemyLargePixmap());
    v->setBossPixmap(svm->getBossPixmap());
    v->setBossPixmap2(svm->getBossPixmapForHp(200));
    v->setBossPixmap3(svm->getBossPixmapForHp(350));
    v->setBossPixmap4(svm->getBossPixmapForHp(500));
    v->setBulletPixmap(svm->getPlayerBulletPixmap());
    v->setEnemyBulletPixmap(svm->getEnemyBulletPixmap());
    v->setBackgroundPixmap(svm->getBackgroundPixmap());
    v->setPowerUpHpPixmap(svm->getPowerUpHpPixmap());
    v->setPowerUpFirePixmap(svm->getPowerUpFirePixmap());
    v->setPowerUpShieldPixmap(svm->getPowerUpShieldPixmap());
    v->setPowerUpStarCorePixmap(svm->getPowerUpStarCorePixmap());
    v->setScorePtr(vm->getScorePtr());
    v->setLivesPtr(vm->getLivesPtr());
    v->setHighScorePtr(vm->getHighScorePtr());
    v->setGameStatePtr(vm->getGameStatePtr());
    v->setWavePtr(vm->getWavePtr());
    v->setWaveDisplayPtr(vm->getWaveDisplayPtr());
    v->setBossHpPtr(vm->getBossHpPtr());
    v->setBossMaxHpPtr(vm->getBossMaxHpPtr());
    v->setCurrentLevelPtr(vm->getCurrentLevelPtr());
    v->setSkillCooldownPtr(vm->getSkillCooldownPtr());
    v->setSkillReadyPtr(vm->isSkillReadyPtr());
    v->setSkillActivePtr(vm->isSkillActivePtr());
    v->setSkillTypePtr(vm->getSkillTypePtr());
    v->setWeaponLevelPtr(vm->getWeaponLevelPtr());
    v->setHasShieldPtr(vm->getHasShieldPtr());
    v->setAircraftNamePtr(vm->getAircraftName());
    v->setLevelClearedPtr(vm->isLevelClearedPtr());
    v->setStarCoresPtr(vm->getUpgradeStarCoresPtr());
    v->setUpgradeFireLevelPtr(vm->getUpgradeFireLevelPtr());
    v->setUpgradeLivesLevelPtr(vm->getUpgradeLivesLevelPtr());
    v->setUpgradeSpeedLevelPtr(vm->getUpgradeSpeedLevelPtr());
    v->setUpgradeCooldownLevelPtr(vm->getUpgradeCooldownLevelPtr());
    v->setMaxUnlockedLevelPtr(vm->getMaxUnlockedLevelPtr());
    v->setLevelSelectMaxUnlocked(7);
    v->setTickCommand(vm->getTickCommand());
    v->setMoveUpCommand(vm->getMoveUpCommand());
    v->setMoveDownCommand(vm->getMoveDownCommand());
    v->setMoveLeftCommand(vm->getMoveLeftCommand());
    v->setMoveRightCommand(vm->getMoveRightCommand());
    v->setStartGameCommand(vm->getStartGameCommand());
    v->setSelectModeCommand(vm->getSelectModeCommand());
    v->setPauseCommand(vm->getPauseCommand());
    v->setStartLevelCommand(vm->getStartLevelCommand());
    v->setSelectLevelCommand(vm->getSelectLevelCommand());
    v->setQuitLevelCommand(vm->getQuitLevelCommand());
    v->setSelectAircraftCommand(vm->getSelectAircraftCommand());
    v->setUseSkillCommand(vm->getUseSkillCommand());
    v->setNavigateCommand(vm->getNavigateCommand());
    v->setUpgradeStatCommand(vm->getUpgradeStatCommand());
    QObject::connect(vm, &GameMapVM::propertyChanged, v, &GameView::onPropertyChanged);
}

static void castSkill(GameView* view) {
    view->setFocus(); QTest::qWait(200);
    QTest::keyClick(view, Qt::Key_Space); QTest::qWait(600);
}

// ═══════════════════════════════════════════════════════════════════════════
TEST_CASE("AutoPlay - all 5 planes demo", "[autoplay]") {
    ensureApp();
    auto* vm   = new GameMapVM();
    auto* svm  = new SpiritVM();
    auto* view = new GameView();
    auto* stack = view->findChild<QStackedWidget*>();
    REQUIRE(stack); REQUIRE(svm->initialize());
    bindAll(view, vm, svm);
    vm->setMaxUnlockedLevel(7);
    view->setLevelSelectMaxUnlocked(7);

    QTimer* timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [vm, view]() {
        if (vm->getGameState() == GameState::Playing) {
            vm->getTickCommand()(0.016f);
            view->setMap(vm->getMap());
        }
    });
    view->resize(800, 600); view->show(); QTest::qWait(300);

    auto wait = [vm](GameState s, int ms = 6000) {
        auto t = QDateTime::currentMSecsSinceEpoch();
        while (QDateTime::currentMSecsSinceEpoch()-t < ms) {
            QTest::qWait(50); if (vm->getGameState() == s) return true;
        }
        return false;
    };

    // ── 从 Menu 进入游戏（选模式→选关→选战机） ────────────────
    std::function<bool(int,int,bool)> enterGame = [&](int levelId, int aircraftIdx, bool endless) -> bool {
        auto* sb = view->findChild<StartScreen*>()->findChild<QPushButton*>();
        if (!sb) return false;
        QTest::mouseClick(sb, Qt::LeftButton); QTest::qWait(400);

        auto btns = view->findChild<ModeSelectScreen*>()->findChildren<QPushButton*>();
        if (btns.size() < 2) return false;
        QTest::mouseClick(endless ? btns[1] : btns[0], Qt::LeftButton);
        QTest::qWait(400);

        if (!endless && !wait(GameState::LevelSelect, 2000)) return false;
        if (!endless) {
            QTest::mouseClick(stack->currentWidget(), Qt::LeftButton, Qt::NoModifier, levelBtn(levelId));
            QTest::qWait(400);
        }
        auto card = acCard(aircraftIdx);
        QTest::mouseClick(stack->currentWidget(), Qt::LeftButton, Qt::NoModifier, card);
        QTest::qWait(300);
        // 确认前换图，避免上一架战机闪现
        auto* px1 = svm->getAircraftPixmap(static_cast<AircraftType>(aircraftIdx));
        if (px1) view->setPlayerPixmap(px1);
        QTest::mouseClick(stack->currentWidget(), Qt::LeftButton, Qt::NoModifier, QPoint(400, 559));
        QTest::qWait(500);
        return wait(GameState::Playing, 4000);
    };

    // ── 快速换下一关 ─────────────────────────────────────────────
    std::function<bool(int,int)> nextLevel = [&](int levelId, int aircraftIdx) -> bool {
        vm->getQuitLevelCommand()();
        if (!wait(GameState::LevelSelect, 3000)) {
            vm->getNavigateCommand()(static_cast<int>(GameState::LevelSelect));
            QTest::qWait(300);
        }
        QTest::mouseClick(stack->currentWidget(), Qt::LeftButton, Qt::NoModifier, levelBtn(levelId));
        QTest::qWait(400);
        auto card2 = acCard(aircraftIdx);
        QTest::mouseClick(stack->currentWidget(), Qt::LeftButton, Qt::NoModifier, card2);
        QTest::qWait(300);
        auto* px2 = svm->getAircraftPixmap(static_cast<AircraftType>(aircraftIdx));
        if (px2) view->setPlayerPixmap(px2);
        QTest::mouseClick(stack->currentWidget(), Qt::LeftButton, Qt::NoModifier, QPoint(400, 559));
        QTest::qWait(500);
        return wait(GameState::Playing, 4000);
    };

    // ═══ 阶段1: 第2关 + 雷霆号（完整BOSS战） ═══
    REQUIRE(enterGame(2, 0, false));
    timer->start(16);
    {
        auto mvR = vm->getMoveRightCommand();
        auto mvL = vm->getMoveLeftCommand();
        QTest::qWait(2000);
        mvR(1); QTest::qWait(1500); mvR(0); QTest::qWait(300);
        mvL(1); QTest::qWait(2000); mvL(0); QTest::qWait(300);
        mvR(1); QTest::qWait(1200); mvR(0); QTest::qWait(300);
        castSkill(view);     // ⚡ 第一次技能
        // 轮询等待技能冷却完毕，一好立刻放第二次
        for (int i = 0; i < 200; i++) {  // 最多等 20s
            QTest::qWait(100);
            if (vm->getGameState() != GameState::Playing) break;
            if (vm->isSkillReady()) {
                // 好了再等 2s，让老师看清清小怪场景
                QTest::qWait(2000);
                view->setFocus();
                QTest::qWait(100);
                QTest::keyClick(view, Qt::Key_Space);
                QTest::qWait(500);
                break;
            }
        }
        QTest::qWait(8000); // 🏆 BOSS出场+出场动画
        if (vm->getGameState() == GameState::Playing) {
            view->setFocus();
            QTest::keyClick(view, Qt::Key_Escape); QTest::qWait(1000);
            QTest::keyClick(view, Qt::Key_Escape); QTest::qWait(200);
        }
    }

    // ═══ 阶段2: 第3关 + 烈焰号 ═══
    REQUIRE(nextLevel(3, 1));
    timer->start(16);
    QTest::qWait(1000);
    castSkill(view);  // 火焰风暴 2.5s
    QTest::qWait(1500);

    // ═══ 阶段3: 第4关 + 冰霜号 ═══
    REQUIRE(nextLevel(4, 2));
    timer->start(16);
    QTest::qWait(1000);
    castSkill(view);  // 极寒护盾 4s
    QTest::qWait(2500);

    // ═══ 阶段4: 第5关 + 幻影号 ═══
    REQUIRE(nextLevel(5, 3));
    timer->start(16);
    QTest::qWait(1000);
    castSkill(view);  // 时空闪避
    QTest::qWait(800);

    // ═══ 阶段5: 无尽模式 + 堡垒号 + 升级面板 ═══
    REQUIRE(enterGame(1, 4, true));
    timer->start(16);
    {
        auto mvR = vm->getMoveRightCommand();
        auto mvL = vm->getMoveLeftCommand();
        QTest::qWait(2000);
        mvR(1); QTest::qWait(1500); mvR(0);
        mvL(1); QTest::qWait(1500); mvL(0);
        QTest::qWait(1000);
        castSkill(view);  // 铁壁守护 3s
        QTest::qWait(1500);

        // 升级面板
        vm->getNavigateCommand()(static_cast<int>(GameState::Upgrade));
        QTest::qWait(500);
        CHECK(wait(GameState::Upgrade));
        QTest::qWait(2500);
    }

    timer->stop();
    QObject::disconnect(timer, &QTimer::timeout, nullptr, nullptr);
    view->hide();
    delete timer;
    SUCCEED("All 5 aircraft demo completed!");
}
