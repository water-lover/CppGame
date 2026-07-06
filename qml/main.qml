import QtQuick 2.15
import QtQuick.Controls 2.15
import CppGame 1.0

// ── 主游戏窗口 ────────────────────────────────────────────────────────────
ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("CppGame")

    // ── 连接 ViewModel（由 C++ 注入） ──────────────────────────────────
    property GameViewModel viewModel: viewModel_

    // ── 游戏画布 ────────────────────────────────────────────────────────
    Rectangle {
        id: gameCanvas
        anchors.fill: parent
        color: "#1a1a2e"

        // ── 分数显示 ──────────────────────────────────────────────────
        Text {
            id: scoreText
            anchors {
                top: parent.top
                right: parent.right
                margins: 20
            }
            font.pixelSize: 28
            font.bold: true
            color: "#e94560"
            text: qsTr("Score: %1").arg(viewModel ? viewModel.score : 0)
        }

        // ── 游戏状态提示 ──────────────────────────────────────────────
        Text {
            id: statusText
            anchors.centerIn: parent
            font.pixelSize: 48
            font.bold: true
            color: "#0f3460"
            text: qsTr("Click to Start")
            opacity: viewModel && viewModel.running ? 0 : 1
            Behavior on opacity { NumberAnimation { duration: 300 } }
        }

        // ── 游戏区域（后续绘制游戏对象） ──────────────────────────────
        Rectangle {
            id: playArea
            anchors {
                centerIn: parent
                margins: 40
            }
            width: parent.width - 80
            height: parent.height - 80
            color: "#16213e"
            border.color: "#0f3460"
            border.width: 2
            radius: 8

            // ── 点击触发游戏操作 ──────────────────────────────────────
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (viewModel) {
                        if (!viewModel.running)
                            viewModel.startGame()
                        else
                            viewModel.addScore(10)
                    }
                }
            }

            // 占位：后续添加游戏精灵、动画等
        }
    }

    // ── 键盘快捷键 ──────────────────────────────────────────────────────
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    Shortcut {
        sequence: "Space"
        onActivated: {
            if (viewModel) {
                if (!viewModel.running)
                    viewModel.startGame()
                else
                    viewModel.addScore(10)
            }
        }
    }
}
