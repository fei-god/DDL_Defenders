#include "CoffeeBlast.h"
#include <cmath>

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
    _radius = 220.0f;
    configureEnergy(100.0f, 28.0f, 18.0f);
    setObjectScale(0.19f);
    setAttackRange(_radius);
    return true;
}

void CoffeeBlast::fire()
{
    if (!canFire()) return;
    if (!hasEnemyInBlastRange()) return;
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

    playBlastEffect(center);

    if (hitAny || _enemyList->empty())
    {
        resetCooldown();
    }
}

bool CoffeeBlast::hasEnemyInBlastRange() const
{
    if (_owner == nullptr || _enemyList == nullptr)
    {
        return false;
    }

    Vec2 center = _owner->getObjectPosition();
    for (auto* enemy : *_enemyList)
    {
        if (!enemy || !enemy->isObjectActive() || !enemy->isRoleAlive())
        {
            continue;
        }

        if (enemy->getObjectPosition().distance(center) <= _radius)
        {
            return true;
        }
    }

    return false;
}

void CoffeeBlast::playBlastEffect(const Vec2& center)
{
    if (_bulletLayer == nullptr)
    {
        return;
    }

    auto effectRoot = Node::create();
    effectRoot->setPosition(center);
    _bulletLayer->addChild(effectRoot, 25);

    auto core = DrawNode::create();
    core->drawSolidCircle(Vec2::ZERO, _radius * 0.26f, 0.0f, 72,
        Color4F(1.0f, 0.82f, 0.36f, 0.46f));
    core->drawCircle(Vec2::ZERO, _radius * 0.34f, 0.0f, 72, false,
        Color4F(1.0f, 0.95f, 0.65f, 0.95f));
    effectRoot->addChild(core);
    core->runAction(Spawn::create(
        ScaleTo::create(0.18f, 1.45f),
        FadeOut::create(0.18f),
        nullptr));

    auto shockRing = DrawNode::create();
    shockRing->drawCircle(Vec2::ZERO, _radius * 0.42f, 0.0f, 96, false,
        Color4F(0.95f, 0.48f, 0.18f, 0.92f));
    shockRing->drawCircle(Vec2::ZERO, _radius * 0.62f, 0.0f, 96, false,
        Color4F(0.62f, 0.30f, 0.12f, 0.65f));
    effectRoot->addChild(shockRing);
    shockRing->runAction(Spawn::create(
        ScaleTo::create(0.24f, 1.8f),
        FadeOut::create(0.24f),
        nullptr));

    auto rays = DrawNode::create();
    for (int i = 0; i < 12; ++i)
    {
        float angle = static_cast<float>(i) * 30.0f;
        float rad = CC_DEGREES_TO_RADIANS(angle);
        Vec2 dir(std::cos(rad), std::sin(rad));
        rays->drawSegment(dir * (_radius * 0.18f), dir * (_radius * 0.82f), 2.5f,
            Color4F(1.0f, 0.72f, 0.28f, 0.80f));
    }
    effectRoot->addChild(rays);
    rays->runAction(Spawn::create(
        ScaleTo::create(0.20f, 1.28f),
        FadeOut::create(0.20f),
        nullptr));

    effectRoot->runAction(Sequence::create(
        DelayTime::create(0.28f),
        RemoveSelf::create(),
        nullptr));
}
