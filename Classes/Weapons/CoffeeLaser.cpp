#include "CoffeeLaser.h"

USING_NS_CC;

//武器特点：伤害高，冷却长，穿透力强，适合对付精英怪和Boss。

CoffeeLaser* CoffeeLaser::create(Player* owner)
{
    CoffeeLaser* weapon = new (std::nothrow) CoffeeLaser();

    if (weapon && weapon->initCoffeeLaser(owner))
    {
        weapon->autorelease();
        return weapon;
    }

    delete weapon;
    return nullptr;
}

bool CoffeeLaser::initCoffeeLaser(Player* owner)
{
    // 咖啡激光伤害更高，但冷却更长。
    if (!initWeapon(
        "CoffeeLaser",     // 武器名字
        "weapon/coffee_gun.png", // 武器本体
        owner,             // 武器所属玩家
        35,                // 攻击力
        1.20f              // 冷却时间
    ))
    {
        return false;
    }

    _bulletSpeed = 900.0f;
    _bulletImagePath = "weapon/coffee_laser.png";
    configureEnergy(100.0f, 34.0f, 14.0f);
    setObjectScale(0.018f);

    return true;
}

void CoffeeLaser::fire()
{
    if (!canFire()) return;
    if (!consumeEnergyForShot()) return;

    Vec2 startPos = _owner->getObjectPosition();
    Vec2 dir = getAimDirection();
    if (dir.lengthSquared() < 0.0001f)
    {
        Enemy* target = findNearestEnemy();
        dir = target ? getDirectionToEnemy(target) : Vec2(1, 0);
    }

    Bullet* bullet = spawnBullet(
        "CoffeeLaserBullet",
        _bulletImagePath,
        startPos,
        dir,
        _bulletSpeed,
        getModifiedAttackPower(),
        0.75f,
        true
    );

    if (bullet != nullptr)
    {
        // 激光可以稍微拉长，看起来更像一束光。
        bullet->setScaleX(0.18f);
        bullet->setScaleY(0.06f);
    }

    resetCooldown();
}
