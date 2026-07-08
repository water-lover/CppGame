import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Particles 2.15

// ═══════════════════════════════════════════════════════════════════════════
//  雷霆战机 — 主窗口
// ═══════════════════════════════════════════════════════════════════════════

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    minimumWidth: 600
    minimumHeight: 450
    title: qsTr("雷霆战机 — Thunder Fighter")
    color: "#0a0a1a"

    // ViewModel 由 C++ 注入为 contextProperty "vm"，直接使用

    // ── 游戏计时器（驱动 C++ 游戏循环，~60 FPS） ─────────────────────
    Timer {
        id: gameTimer
        interval: 16
        running: vm.playing
        repeat: true
        onTriggered: vm.tick(0.016)
    }

    // ── 全局键盘处理（始终接受焦点，不影响按钮点击） ──────────────
    Item {
        id: keyboardHandler
        anchors.fill: parent
        focus: true

        Keys.onPressed: {
            if (!vm.playing) return;
            switch (event.key) {
                case Qt.Key_W: case Qt.Key_Up:    vm.moveUp(true);    event.accepted = true; break;
                case Qt.Key_S: case Qt.Key_Down:  vm.moveDown(true);  event.accepted = true; break;
                case Qt.Key_A: case Qt.Key_Left:  vm.moveLeft(true);  event.accepted = true; break;
                case Qt.Key_D: case Qt.Key_Right: vm.moveRight(true); event.accepted = true; break;
            }
        }
        Keys.onReleased: {
            switch (event.key) {
                case Qt.Key_W: case Qt.Key_Up:    vm.moveUp(false);    break;
                case Qt.Key_S: case Qt.Key_Down:  vm.moveDown(false);  break;
                case Qt.Key_A: case Qt.Key_Left:  vm.moveLeft(false);  break;
                case Qt.Key_D: case Qt.Key_Right: vm.moveRight(false); break;
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    //  开始界面
    // ═══════════════════════════════════════════════════════════════════
    Item {
        id: startScreen
        anchors.fill: parent
        visible: !vm.playing && !vm.gameOver

        Rectangle {
            anchors.fill: parent
            color: "#0a0a1a"

            Image {
                anchors.fill: parent
                source: "qrc:/images/background"
                fillMode: Image.PreserveAspectCrop
                opacity: 0.3
            }
        }

        Column {
            anchors.centerIn: parent
            spacing: 30

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "⚡ 雷霆战机 ⚡"
                font.pixelSize: 48
                font.bold: true
                color: "#FFD700"
                style: Text.Raised
                styleColor: "#8B0000"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Thunder Fighter"
                font.pixelSize: 20
                color: "#87CEEB"
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "🚀 开始游戏"
                font.pixelSize: 24
                font.bold: true
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 240
                    implicitHeight: 60
                    radius: 12
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#FF4500" }
                        GradientStop { position: 1.0; color: "#8B0000" }
                    }
                }
                onClicked: {
                    vm.startGame()
                    keyboardHandler.forceActiveFocus()
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "WASD 移动 · 自动射击"
                font.pixelSize: 14
                color: "#888888"
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    //  游戏画面
    // ═══════════════════════════════════════════════════════════════════
    Item {
        id: gameCanvas
        anchors.fill: parent
        visible: vm.playing || vm.gameOver

        // ── 背景 ──────────────────────────────────────────────────────
        Rectangle {
            anchors.fill: parent
            color: "#0a0a1a"
        }

        Image {
            anchors.fill: parent
            source: "qrc:/images/background"
            fillMode: Image.PreserveAspectCrop
            opacity: 0.4
        }

        // ── 星空粒子背景 ──────────────────────────────────────────────
        ParticleSystem {
            id: starSystem
            anchors.fill: parent
            running: vm.playing
        }

        Emitter {
            system: starSystem
            anchors.fill: parent
            lifeSpan: 4000
            emitRate: 8
            size: 3
            velocity: PointDirection { y: 40; yVariation: 10 }
            acceleration: PointDirection { y: 20 }
        }

        // ── 敌机渲染 ────────────────────────────────────────────────
        Repeater {
            model: vm.enemies
            delegate: EnemyPlane {
                x: modelData.x * parent.width  - width  / 2
                y: modelData.y * parent.height - height / 2
            }
        }

        // ── 子弹渲染 ────────────────────────────────────────────────
        Repeater {
            model: vm.bullets
            delegate: BulletShape {
                x: modelData.x * parent.width  - width  / 2
                y: modelData.y * parent.height - height / 2
            }
        }

        // ── 玩家飞机 ────────────────────────────────────────────────
        PlayerPlane {
            id: playerPlane
            visible: vm.playing
            x: vm.playerX * parent.width  - width  / 2
            y: vm.playerY * parent.height - height / 2
        }

        // ── HUD ──────────────────────────────────────────────────────
        HUD {
            id: hud
            visible: vm.playing
        }

        // ── Game Over 画面 ──────────────────────────────────────────
        GameOverScreen {
            id: gameOverScreen
            visible: vm.gameOver
        }
    }

    // 窗口加载完成后将焦点交给键盘处理器
    Component.onCompleted: keyboardHandler.forceActiveFocus()
}
