#include "DeskLampLaser.h"

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
    Bullet* bullet = spawnBullet("DeskLampLaserBullet", _bulletImagePath,
        _owner->getObjectPosition(), dir, _bulletSpeed, damage, 1.25f, true);
    if (bullet)
    {
        bullet->setScaleX(0.22f);
        bullet->setScaleY(0.09f);
    }
    resetCooldown();
}
