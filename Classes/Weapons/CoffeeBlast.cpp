#include "CoffeeBlast.h"

USING_NS_CC;

CoffeeBlast* CoffeeBlast::create(Player* owner)
{
    CoffeeBlast* weapon = new (std::nothrow) CoffeeBlast();
    if (weapon && weapon->initCoffeeBlast(owner))
    {
        weapon->autorelease();
        return weapon;
    }
    CC_SAFE_DELETE(weapon);
    return nullptr;
}

bool CoffeeBlast::initCoffeeBlast(Player* owner)
{
    if (!initWeapon("CoffeeBlast", "weapon/coffee_blast_sprite.png", owner, 38, 1.05f))
    {
        return false;
    }
    _radius = 150.0f;
    configureEnergy(100.0f, 28.0f, 18.0f);
    setObjectScale(0.19f);
    setAttackRange(185.0f);
    return true;
}

void CoffeeBlast::fire()
{
    if (!canFire()) return;
    if (!consumeEnergyForShot()) return;

    Vec2 center = _owner->getObjectPosition();
    int damage = getModifiedAttackPower();
    bool hitAny = false;

    for (auto* enemy : *_enemyList)
    {
        if (!enemy || !enemy->isObjectActive() || !enemy->isRoleAlive())
        {
            continue;
        }

        if (enemy->getObjectPosition().distance(center) <= _radius)
        {
            enemy->takeDamage(damage);
            hitAny = true;
        }
    }

    auto ring = DrawNode::create();
    ring->drawCircle(Vec2::ZERO, _radius, 0.0f, 96, false,
        Color4F(0.95f, 0.55f, 0.25f, 0.85f));
    ring->setPosition(center);
    _bulletLayer->addChild(ring, 20);
    ring->runAction(Sequence::create(FadeOut::create(0.18f), RemoveSelf::create(), nullptr));

    if (hitAny || _enemyList->empty())
    {
        resetCooldown();
    }
}
