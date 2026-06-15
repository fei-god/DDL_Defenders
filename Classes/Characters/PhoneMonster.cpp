#include "PhoneMonster.h"

USING_NS_CC;

PhoneMonster* PhoneMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    PhoneMonster* monster = new (std::nothrow) PhoneMonster();
    if (monster && monster->initPhoneMonster(imagePath, startPosition, target))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool PhoneMonster::initPhoneMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    bool ok = initEnemy("PhoneMonster", imagePath, startPosition,
        20, 145.0f, 0, 6, 46.0f, 35);
    if (!ok) return false;

    setTargetPlayer(target);
    _attackCooldownMax = 0.65f;
    _zigzagTimer = 0.0f;
    _zigzagSign = 1.0f;
    return true;
}

void PhoneMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    _zigzagTimer -= dt;
    if (_zigzagTimer <= 0.0f)
    {
        _zigzagTimer = 0.35f;
        _zigzagSign = -_zigzagSign;
    }

    Vec2 dir = _targetPlayer->getPosition() - getPosition();
    if (dir.lengthSquared() < 0.0001f) return;
    dir.normalize();

    Vec2 side(-dir.y, dir.x);
    Vec2 blended = (dir + side * 0.35f * _zigzagSign).getNormalized();
    setDirection(blended);
    setPosition(getPosition() + blended * getSpeed() * dt);
}

void PhoneMonster::attack()
{
    if (_targetPlayer == nullptr) return;

    _targetPlayer->takeDamage(_attackDamage);
    if (_targetPlayer->getMoodSystem())
    {
        _targetPlayer->getMoodSystem()->changeMood(MoodType::Irritable, 2.0f);
    }
}
