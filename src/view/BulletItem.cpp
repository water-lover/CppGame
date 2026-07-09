#include "view/BulletItem.hpp"
#include "common/Constants.hpp"

/// 子弹图形项 — 支持玩家/敌方颜色区分
///
/// 迭代 3 扩展：
///   - PlayerBullet: 蓝色激光 (laserBlue16.png)
///   - EnemyBullet:  红色子弹 (laserRed01.png)
///
/// 当前由 GameScene::drawForeground 根据 ActorType 选择对应图片，
/// 此类为后续独立 QGraphicsItem 方案预留。
// 构造器已在头文件中 inline 实现
