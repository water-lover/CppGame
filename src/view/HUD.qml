import QtQuick 2.15
import QtQuick.Controls 2.15

// ═══════════════════════════════════════════════════════════════════════════
//  抬头显示 — 分数 / 生命 / 波次
// ═══════════════════════════════════════════════════════════════════════════
Item {
    anchors.fill: parent

    // ── 分数 ──────────────────────────────────────────────────────────
    Text {
        id: scoreText
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        text: "分数: " + vm.score
        font.pixelSize: 22
        font.bold: true
        color: "#FFFFFF"
        style: Text.Raised
        styleColor: "#000000"
    }

    // ── 最高分 ───────────────────────────────────────────────────────
    Text {
        anchors.top: scoreText.bottom
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.topMargin: 4
        text: "最高分: " + vm.highScore
        font.pixelSize: 14
        color: "#FFD700"
    }

    // ── 生命 ──────────────────────────────────────────────────────────
    Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        spacing: 6

        Repeater {
            model: vm.lives
            Text {
                text: "❤️"
                font.pixelSize: 24
            }
        }
    }

    // ── 波次 ──────────────────────────────────────────────────────────
    Text {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 16
        text: "波次 " + vm.wave
        font.pixelSize: 18
        color: "#87CEEB"
        visible: vm.wave > 0
    }
}
