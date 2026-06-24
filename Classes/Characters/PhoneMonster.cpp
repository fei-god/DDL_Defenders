#include "PhoneMonster.h"

USING_NS_CC;

PhoneMonster* PhoneMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    PhoneMonster* monster = new (std::nothrow) PhoneMonster();
    if (monster && monster->initPhoneMonster(imagePath, startPosition, target, waveLevel))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool PhoneMonster::initPhoneMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    // Stats scale with wave level
    int scaledHp = 20 + waveLevel * 3;
    float scaledSpeed = 145.0f + waveLevel * 2.0f;
    int scaledAtk = 6 + waveLevel * 1;
    int scaledExp = 35 + waveLevel * 5;
    bool ok = initEnemy("PhoneMonster", imagePath, startPosition,
        scaledHp, scaledSpeed, 0, scaledAtk, 46.0f, scaledExp);
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
    _targetPlayer->changeMood(MoodType::Irritable, 2.0f);
}
