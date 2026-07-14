#include "viewmodel/SpiritVM.hpp"
#include "resource/AssetManager.hpp"

bool SpiritVM::initialize() {
    AssetManager& assets = AssetManager::instance();

    // 5 架战机图片
    setAircraftPixmap(AircraftType::Thunder,  assets.getImage("thunderShip"));
    setAircraftPixmap(AircraftType::Flame,    assets.getImage("flameShip"));
    setAircraftPixmap(AircraftType::Frost,    assets.getImage("frostShip"));
    setAircraftPixmap(AircraftType::Phantom,  assets.getImage("phantomShip"));
    setAircraftPixmap(AircraftType::Fortress, assets.getImage("fortressShip"));

    setEnemySmallPixmap(assets.getImage("enemySmall"));
    setPlayerBulletPixmap(assets.getImage("playerBullet"));
    setEnemyBulletPixmap(assets.getImage("enemyBullet"));
    setBackgroundPixmap(assets.getImage("background"));
    setStarfieldFarPixmap(assets.getImage("starfieldFar"));
    setStarfieldNearPixmap(assets.getImage("starfieldNear"));

    // 预缩放星域图到 800×600（空间背景可拉伸，忽略宽高比）
    if (m_pStarfieldFar && !m_pStarfieldFar->isNull())
        m_starfieldFarScaled = m_pStarfieldFar->scaled(800, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (m_pStarfieldNear && !m_pStarfieldNear->isNull())
        m_starfieldNearScaled = m_pStarfieldNear->scaled(800, 600, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setEnemyMediumPixmap(assets.getImage("enemyMedium"));
    setEnemyLargePixmap(assets.getImage("enemyLarge"));
    setBossPixmap(assets.getImage("bossShip"));
    setBossPixmap2(assets.getImage("bossShip2"));
    setBossPixmap3(assets.getImage("bossShip3"));
    setBossPixmap4(assets.getImage("bossShip4"));
    setEnemyBulletPixmap(assets.getImage("enemyBullet"));
    setPowerUpHpPixmap(assets.getImage("powerUpHp"));
    setPowerUpFirePixmap(assets.getImage("powerUpFire"));
    setPowerUpShieldPixmap(assets.getImage("powerUpShield"));
    setPowerUpStarCorePixmap(assets.getImage("powerUpStarCore"));

    return true;
}

void SpiritVM::setAircraftPixmap(AircraftType type, const QPixmap* p) noexcept {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 5) {
        m_aircraftPixmaps[idx] = p;
    }
}

const QPixmap* SpiritVM::getAircraftPixmap(AircraftType type) const noexcept {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 5) {
        return m_aircraftPixmaps[idx];
    }
    return nullptr;
}

const QPixmap* SpiritVM::getBossPixmapForHp(int maxHp) const noexcept {
    if (maxHp <= 250) {
        return m_pBossImg2 ? m_pBossImg2 : m_pBossImg;   // 中型BOSS
    } else if (maxHp <= 400) {
        return m_pBossImg3 ? m_pBossImg3 : m_pBossImg;   // 重型BOSS
    } else {
        return m_pBossImg4 ? m_pBossImg4 : m_pBossImg;   // 装甲BOSS
    }
}
