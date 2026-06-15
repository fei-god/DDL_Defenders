#include "CoffeeGun.h"
#include <algorithm>

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
    if (!initWeapon("CoffeeGun", "weapon/coffee_gun.png", owner, 22, 0.28f))
    {
        return false;
    }

    _bulletSpeed = 680.0f;
    _bulletImagePath = "weapon/coffee_laser.png";
    configureEnergy(100.0f, 12.0f, 28.0f);
    setObjectScale(0.018f);
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

    Bullet* bullet = spawnBullet("CoffeeGunBullet", _bulletImagePath,
        _owner->getObjectPosition(), dir, _bulletSpeed,
        getModifiedAttackPower(), 1.1f, false);
    if (bullet)
    {
        auto size = bullet->getContentSize();
        if (size.width > 0.0f && size.height > 0.0f)
        {
            bullet->setScale(std::min(18.0f / size.width, 18.0f / size.height));
        }
    }

    resetCooldown();
}
