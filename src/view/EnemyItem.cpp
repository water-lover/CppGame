#include "view/EnemyItem.hpp"
#include "view/ViewConstants.hpp"

/// 敌机图形项 — 支持按类型设置不同尺寸
///
/// 迭代 3 扩展：根据 ActorType 自动选择合适缩放
///   - EnemySmall:  1.0x  (ENEMY_SIZE)
///   - EnemyMedium: 1.4x
///   - EnemyLarge:  1.8x
///   - Boss:        3.0x
///
/// 当前由 GameScene::drawForeground 直接绘制，
/// 此类为后续独立 QGraphicsItem 方案预留。
// 构造器已在头文件中 inline 实现
