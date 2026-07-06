#include "CoffeeLaser.h"

#include <cmath>

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
        "weapon/coffee_gun_sprite.png", // 武器本体
        owner,             // 武器所属玩家
        35,                // 攻击力
        1.20f              // 冷却时间
    ))
    {
        return false;
    }

    _bulletSpeed = 1800.0f;
    _bulletImagePath = "weapon/coffee_bullet_sprite.png";
    configureEnergy(100.0f, 34.0f, 14.0f);
    setObjectScale(0.19f);
    setAttackRange(760.0f);

    return true;
}

void CoffeeLaser::fire()
{
    if (!canFire()) return;
    if (!consumeEnergyForShot()) return;

    Vec2 dir = getAimDirection();
    if (dir.lengthSquared() < 0.0001f)
    {
        Enemy* target = findNearestEnemy();
        dir = target ? getDirectionToEnemy(target) : Vec2(1, 0);
    }

    int projectileCount = 1 + getProjectileCountBonus();
    float spread = projectileCount > 1 ? 6.0f : 0.0f;
    for (int i = 0; i < projectileCount; ++i)
    {
        float angle = (i - (projectileCount - 1) * 0.5f) * spread;
        float rad = CC_DEGREES_TO_RADIANS(angle);
        Vec2 shotDir(dir.x * std::cos(rad) - dir.y * std::sin(rad),
            dir.x * std::sin(rad) + dir.y * std::cos(rad));

        Bullet* bullet = spawnBullet(
            "CoffeeLaserBullet",
            _bulletImagePath,
            getMuzzlePosition(shotDir),
            shotDir,
            _bulletSpeed,
            getModifiedAttackPower(),
            0.75f,
            true
        );

        if (bullet != nullptr)
        {
            bullet->setScale(0.18f);
        }
    }

    resetCooldown();
}
