#include "DeskLampLaser.h"

#include <cmath>

USING_NS_CC;

DeskLampLaser* DeskLampLaser::create(Player* owner)
{
    DeskLampLaser* weapon = new (std::nothrow) DeskLampLaser();
    if (weapon && weapon->initDeskLampLaser(owner))
    {
        weapon->autorelease();
        return weapon;
    }
    CC_SAFE_DELETE(weapon);
    return nullptr;
}

bool DeskLampLaser::initDeskLampLaser(Player* owner)
{
    if (!initWeapon("DeskLampLaser", "weapon/desk_lamp_weapon_sprite.png", owner, 42, 0.75f))
    {
        return false;
    }
    _bulletSpeed = 900.0f;
    _bulletImagePath = "weapon/desk_lamp_laser_sprite.png";
    configureEnergy(100.0f, 30.0f, 16.0f);
    setObjectScale(0.095f);
    return true;
}

void DeskLampLaser::fire()
{
    if (!canFire()) return;
    if (!consumeEnergyForShot()) return;

    Vec2 dir = getAimDirection();
    if (dir.lengthSquared() < 0.0001f)
    {
        Enemy* target = findNearestEnemy();
        dir = target ? getDirectionToEnemy(target) : Vec2(1, 0);
    }

    int damage = getModifiedAttackPower();
    int projectileCount = 1 + getProjectileCountBonus();
    float spread = projectileCount > 1 ? 5.0f : 0.0f;
    for (int i = 0; i < projectileCount; ++i)
    {
        float angle = (i - (projectileCount - 1) * 0.5f) * spread;
        float rad = CC_DEGREES_TO_RADIANS(angle);
        Vec2 shotDir(dir.x * std::cos(rad) - dir.y * std::sin(rad),
            dir.x * std::sin(rad) + dir.y * std::cos(rad));

        Bullet* bullet = spawnBullet("DeskLampLaserBullet", _bulletImagePath,
            _owner->getObjectPosition(), shotDir, _bulletSpeed, damage, 1.25f, true);
        if (bullet)
        {
            bullet->setScaleX(0.22f);
            bullet->setScaleY(0.09f);
        }
    }
    resetCooldown();
}
