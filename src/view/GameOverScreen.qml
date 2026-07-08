import QtQuick 2.15
import QtQuick.Controls 2.15

// ═══════════════════════════════════════════════════════════════════════════
//  游戏结束界面
// ═══════════════════════════════════════════════════════════════════════════
Rectangle {
    anchors.fill: parent
    color: "#88000000"  // 半透明遮罩

    Column {
        anchors.centerIn: parent
        spacing: 24

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "💥 游戏结束 💥"
            font.pixelSize: 42
            font.bold: true
            color: "#FF4444"
            style: Text.Raised
            styleColor: "#8B0000"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "最终得分: " + vm.score
            font.pixelSize: 28
            color: "#FFFFFF"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "最高分: " + vm.highScore
            font.pixelSize: 20
            color: "#FFD700"
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "🔄 再来一局"
            font.pixelSize: 20
            font.bold: true
            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 50
                radius: 10
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#FF4500" }
                    GradientStop { position: 1.0; color: "#8B0000" }
                }
            }
            onClicked: vm.startGame()
        }
    }
}
