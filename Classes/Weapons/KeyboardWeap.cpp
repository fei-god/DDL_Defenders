#include "KeyboardWeap.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

KeyboardWeap* KeyboardWeap::create(Player* owner)
{
    KeyboardWeap* weapon = new (std::nothrow) KeyboardWeap();
    if (weapon && weapon->initKeyboardWeap(owner))
    {
        weapon->autorelease();
        return weapon;
    }

    CC_SAFE_DELETE(weapon);
    return nullptr;
}

bool KeyboardWeap::initKeyboardWeap(Player* owner)
{
    if (!initWeapon("KeyboardWeap", "weapon/keyboard_weapon_sprite.png", owner, 48, 0.9f))
    {
        return false;
    }

    _bulletSpeed = 1160.0f;
    _bulletImagePath = "weapon/keyboard_weap_bolt_sprite.png";
    configureEnergy(100.0f, 32.0f, 15.0f);
    setObjectScale(0.19f);
    setAttackRange(640.0f);
    return true;
}

void KeyboardWeap::fire()
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
    float spread = projectileCount > 1 ? 7.0f : 0.0f;
    for (int i = 0; i < projectileCount; ++i)
    {
        float angle = (i - (projectileCount - 1) * 0.5f) * spread;
        float rad = CC_DEGREES_TO_RADIANS(angle);
        Vec2 shotDir(dir.x * std::cos(rad) - dir.y * std::sin(rad),
            dir.x * std::sin(rad) + dir.y * std::cos(rad));

        Bullet* bullet = spawnBullet("KeyboardWeapBullet", _bulletImagePath,
            getMuzzlePosition(shotDir), shotDir, _bulletSpeed,
            getModifiedAttackPower(), 1.0f, true);
        if (bullet)
        {
            auto size = bullet->getContentSize();
            if (size.width > 0.0f && size.height > 0.0f)
            {
                bullet->setScale(std::min(72.0f / size.width, 28.0f / size.height));
            }
        }
    }

    resetCooldown();
}
