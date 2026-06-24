#include "DDLMonster.h"

USING_NS_CC;

DDLMonster* DDLMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    DDLMonster* monster = new DDLMonster();
    if (monster && monster->initDDLMonster(imagePath, startPosition, target, waveLevel))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool DDLMonster::initDDLMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    // Charge-type monster: medium HP, faster speed, medium attack
    // Stats scale with wave level
    int scaledHp = 24 + waveLevel * 5;
    float scaledSpeed = 115.0f + waveLevel * 3.0f;
    int scaledAtk = 12 + waveLevel * 2;
    int scaledExp = 50 + waveLevel * 8;
    bool ok = initEnemy("DDLMonster",
        imagePath,
        startPosition,
        scaledHp,
        scaledSpeed,
        0,
        scaledAtk,
        64.0f,
        scaledExp);
    if (!ok) return false;

    setTargetPlayer(target);
    _chargeCooldown = 0.0f;
    _isCharging = false;
    _chargeDirection = Vec2::ZERO;
    return true;
}

void DDLMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    if (_chargeCooldown > 0.0f)
        _chargeCooldown -= dt;

    if (_isCharging)
    {
        float chargeSpeed = getSpeed() * 2.5f;
        Vec2 newPos = getPosition() + _chargeDirection * chargeSpeed * dt;
        setPosition(newPos);

        if (_chargeCooldown <= 0.0f)
        {
            _isCharging = false;
            _chargeCooldown = 1.5f;
        }
        return;
    }
    else
    {
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
        if (_chargeCooldown <= 0.0f && distToPlayer < 150.0f)
        {
            _isCharging = true;
            _chargeDirection = dir;
            _chargeCooldown = 0.4f;
        }
    }
}

void DDLMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    _targetPlayer->takeDamage(_attackDamage);
}
