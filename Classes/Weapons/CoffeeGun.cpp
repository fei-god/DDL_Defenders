#include "CoffeeGun.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

CoffeeGun* CoffeeGun::create(Player* owner)
{
    CoffeeGun* weapon = new (std::nothrow) CoffeeGun();
    if (weapon && weapon->initCoffeeGun(owner))
    {
        weapon->autorelease();
        return weapon;
    }

    CC_SAFE_DELETE(weapon);
    return nullptr;
}

bool CoffeeGun::initCoffeeGun(Player* owner)
{
    if (!initWeapon("CoffeeGun", "weapon/coffee_gun_sprite.png", owner, 22, 0.28f))
    {
        return false;
    }

    _bulletSpeed = 1360.0f;
    _bulletImagePath = "weapon/coffee_bullet_sprite.png";
    configureEnergy(100.0f, 12.0f, 28.0f);
    setObjectScale(0.19f);
    setAttackRange(560.0f);
    return true;
}

void CoffeeGun::fire()
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
    float spread = projectileCount > 1 ? 9.0f : 0.0f;
    for (int i = 0; i < projectileCount; ++i)
    {
        float angle = (i - (projectileCount - 1) * 0.5f) * spread;
        float rad = CC_DEGREES_TO_RADIANS(angle);
        Vec2 shotDir(dir.x * std::cos(rad) - dir.y * std::sin(rad),
            dir.x * std::sin(rad) + dir.y * std::cos(rad));

        Bullet* bullet = spawnBullet("CoffeeGunBullet", _bulletImagePath,
            getMuzzlePosition(shotDir), shotDir, _bulletSpeed,
            getModifiedAttackPower(), 1.1f, false);
        if (bullet)
        {
            auto size = bullet->getContentSize();
            if (size.width > 0.0f && size.height > 0.0f)
            {
                bullet->setScale(std::min(18.0f / size.width, 18.0f / size.height));
            }
        }
    }

    resetCooldown();
}
