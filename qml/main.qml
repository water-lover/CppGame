import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Particles 2.15

// ═══════════════════════════════════════════════════════════════════════════
//  CppGame — 打靶小游戏
//  技术栈：Qt Quick / QML + C++ MVVM
//  特性：粒子星空背景、动画目标、计分升级、倒计时
// ═══════════════════════════════════════════════════════════════════════════

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    minimumWidth: 600
    minimumHeight: 450
    title: qsTr("CppGame — 打靶小游戏")
    color: "#0a0a1a"

    // ── ViewModel 绑定（由 C++ 注入） ──────────────────────────────────
    property var vm: viewModel_

    // ── 游戏计时器（驱动 C++ 游戏循环） ────────────────────────────────
    Timer {
        interval: 16  // ~60 FPS
        running: true
        repeat: true
        onTriggered: {
            if (vm) vm.tick(0.016)
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    //  主游戏画布
    // ═══════════════════════════════════════════════════════════════════
    Item {
        id: gameCanvas
        anchors.fill: parent

        // ── 粒子星空背景 ──────────────────────────────────────────────
        ParticleSystem {
            id: starSystem
            anchors.fill: parent
            running: true
        }

        Emitter {
            system: starSystem
            anchors.fill: parent
            lifeSpan: 4000
            emitRate: 20
            size: 2
            sizeVariation: 1.5
            velocity: PointDirection {
                yVariation: 30
                xVariation: 5
            }
            acceleration: PointDirection { y: 20 }
        }

        // ── 顶部信息栏 ────────────────────────────────────────────────
        Rectangle {
            id: headerBar
            anchors {
                top: parent.top; left: parent.left; right: parent.right
                margins: 10
            }
            height: 50
            color: "#1a1a3e"
            radius: 8
            opacity: 0.85

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 20

                // 分数
                Row {
                    spacing: 5
                    anchors.verticalCenter: parent.verticalCenter
                    Image {
                        source: "data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='%23f0c040'><polygon points='12,2 15.09,8.26 22,9.27 17,14.14 18.18,21.02 12,17.77 5.82,21.02 7,14.14 2,9.27 8.91,8.26'/></svg>"
                        anchors.verticalCenter: parent.verticalCenter
                        width: 20; height: 20
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: 22
                        font.bold: true
                        color: "#f0c040"
                        text: vm ? vm.score : "0"
                    }
                }

                // 等级
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                    color: "#80d0ff"
                    text: qsTr("Lv.%1").arg(vm ? vm.level : 1)
                }

                // 倒计时
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 120; height: 30
                    radius: 15
                    color: vm && vm.running && vm.timeLeft < 10 ? "#8a2020" : "#2a2a5a"
                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: 16
                        font.bold: true
                        color: vm && vm.running && vm.timeLeft < 10 ? "#ff4444" : "#ffffff"
                        text: {
                            if (!vm) return "00:00"
                            var sec = Math.max(0, Math.ceil(vm.timeLeft))
                            var m = Math.floor(sec / 60)
                            var s = sec % 60
                            return ("0" + m).slice(-1) + ":" + ("0" + s).slice(-2)
                        }
                    }
                }

                // 游戏状态提示
                Item { width: 10; height: 1 }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 14
                    color: vm && vm.running ? "#60d060" : "#e04040"
                    text: vm && vm.running ? qsTr("▶ 游戏中") : qsTr("■ 已暂停")
                }
            }
        }

        // ── 游戏区域（480x480 居中） ──────────────────────────────────
        Rectangle {
            id: playField
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height) * 0.75
            height: width
            radius: 12
            color: "#12122a"
            border.color: "#2a2a5a"
            border.width: 2

            // ── 点击目标 ──────────────────────────────────────────────
            Rectangle {
                id: target
                width: parent.width * (vm ? vm.targetSize : 0.08)
                height: width
                radius: width / 2
                color: {
                    if (!vm || !vm.running) return "transparent"
                    if (vm.timeLeft < 10) return "#ff4444"
                    if (vm.level >= 5) return "#ff8800"
                    return "#ff6600"
                }
                opacity: vm && vm.running ? 1 : 0

                x: parent.width * (vm ? vm.targetX : 0.5) - width / 2
                y: parent.height * (vm ? vm.targetY : 0.5) - height / 2

                Behavior on x { SpringAnimation { spring: 3; damping: 0.3; mass: 0.5 } }
                Behavior on y { SpringAnimation { spring: 3; damping: 0.3; mass: 0.5 } }
                Behavior on opacity { NumberAnimation { duration: 200 } }
                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutBack } }
                Behavior on height { NumberAnimation { duration: 300; easing.type: Easing.OutBack } }

                // 光晕
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 1.6
                    height: width
                    radius: width / 2
                    color: parent.color
                    opacity: 0.15
                    Behavior on color { ColorAnimation { duration: 300 } }
                }

                // 十字准心
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.6
                    height: 2
                    color: "white"
                    opacity: 0.6
                }
                Rectangle {
                    anchors.centerIn: parent
                    width: 2
                    height: parent.height * 0.6
                    color: "white"
                    opacity: 0.6
                }

                // 点击波纹动画
                NumberAnimation {
                    id: hitAnim
                    target: target
                    property: "scale"
                    from: 1.0; to: 1.3
                    duration: 80
                    easing.type: Easing.OutQuad
                }

                // 点击事件
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -10
                    onClicked: {
                        if (vm && vm.running) {
                            vm.clickTarget(vm.targetX, vm.targetY)
                            hitAnim.start()
                            spawnParticles(mouseX + target.x, mouseY + target.y)
                        }
                    }
                }
            }

            // ── 粒子爆发特效（点击命中） ──────────────────────────────
            function spawnParticles(x, y) {
                var component = Qt.createQmlObject(
                    'import QtQuick 2.15; import QtQuick.Particles 2.15; ' +
                    'ParticleSystem { id: ps' + Date.now() + ' }',
                    playField);
                // 用简单的方式做粒子
                for (var i = 0; i < 5; i++) {
                    var p = Qt.createQmlObject(
                        'import QtQuick 2.15; Rectangle { ' +
                        'width: 4; height: 4; radius: 2; color: "#ff6600"; ' +
                        'x: ' + x + '; y: ' + y + '; ' +
                        'NumberAnimation on x { from: ' + x + '; to: ' + (x + Math.random()*60-30) + '; duration: 400 } ' +
                        'NumberAnimation on y { from: ' + y + '; to: ' + (y + Math.random()*60-30) + '; duration: 400 } ' +
                        'NumberAnimation on opacity { from: 1; to: 0; duration: 400 } ' +
                        'Component.onCompleted: destroy(500) }',
                        playField);
                }
            }

            // ── 开始提示 ──────────────────────────────────────────────
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                color: "#4060a0"
                text: qsTr("点击目标得分 | 30秒倒计时")
                opacity: vm && vm.running ? 0 : 0.6
                Behavior on opacity { NumberAnimation { duration: 300 } }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    //  覆盖层：开始 / 结束界面
    // ═══════════════════════════════════════════════════════════════════
    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "#0a0a1a"
        opacity: 0.92
        visible: !vm || !vm.running

        Column {
            anchors.centerIn: parent
            spacing: 20

            // 标题
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 48
                font.bold: true
                color: "#ff6600"
                text: qsTr("🎯 CppGame")
            }

            // 副标题（显示分数或提示）
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 20
                color: "#80a0d0"
                text: {
                    if (!vm) return qsTr("加载中...")
                    if (vm.score > 0) return qsTr("最终得分: %1  |  等级: %2").arg(vm.score).arg(vm.level)
                    return qsTr("点击开始，30秒内尽可能多得分!")
                }
            }

            // 开始按钮
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 200; height: 50
                radius: 25
                color: "#ff6600"
                opacity: 0.9

                Text {
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    font.bold: true
                    color: "white"
                    text: vm && vm.score > 0 ? qsTr("🔄 再来一局") : qsTr("▶ 开始游戏")
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: { if (vm) vm.startGame() }
                }

                // 呼吸动画
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    NumberAnimation { from: 0.7; to: 1.0; duration: 1200; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.0; to: 0.7; duration: 1200; easing.type: Easing.InOutQuad }
                }
            }

            // 快捷键提示
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 12
                color: "#4060a0"
                text: qsTr("按 Space 开始  | 按 Esc 退出")
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════
    //  快捷键
    // ═══════════════════════════════════════════════════════════════════
        Shortcut {
            sequence: "Space"
            onActivated: {
                if (vm && !vm.running) vm.startGame()
            }
        }
        Shortcut {
            sequence: "Escape"
            onActivated: Qt.quit()
        }
        Shortcut {
            sequence: StandardKey.Quit
            onActivated: Qt.quit()
        }
    }
