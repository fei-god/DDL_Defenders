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
        "",                // 武器本体
        owner,             // 武器所属玩家
        35,                // 攻击力
        1.20f              // 冷却时间
    ))
    {
        return false;
    }

    _bulletSpeed = 900.0f;
    _bulletImagePath = "weapon/coffee_laser.png";

    return true;
}

void CoffeeLaser::fire()
{
    Enemy* target = findNearestEnemy();

    if (target == nullptr)
    {
        return;
    }

    Vec2 startPos = _owner->getObjectPosition();
    Vec2 dir = getDirectionToEnemy(target);

    Bullet* bullet = Bullet::createBullet(
        "CoffeeLaserBullet",
        _bulletImagePath,
        startPos,
        dir,
        _bulletSpeed,
        _attackPower,
        0.75f,
        true
    );

    if (bullet != nullptr)
    {
        // 激光可以稍微拉长，看起来更像一束光。
        bullet->setScaleX(2.2f);
        bullet->setScaleY(0.7f);

        _bulletLayer->addChild(bullet);
        _bulletList->push_back(bullet);
    }

    resetCooldown();
}