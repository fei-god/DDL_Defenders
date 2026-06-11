#include "DDLMonster.h"

USING_NS_CC;

DDLMonster* DDLMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    DDLMonster* monster = new DDLMonster();
    if (monster && monster->initDDLMonster(imagePath, startPosition, target))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool DDLMonster::initDDLMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    // 冲锋型怪物：中等血量，较快速度，中等攻击
    bool ok = initEnemy("DDLMonster",
        imagePath,
        startPosition,
        50,        // maxHp       (降低：60→50)
        80.0f,     // speed       (降低：200→80，之前太快了)
        5,         // defense     (降低：10→5)
        20,        // attackDamage (降低：35→20)
        55.0f,     // attackRange  (降低：60→55)
        50);       // expReward    (降低：80→50)
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

    // 冷却计时
    if (_chargeCooldown > 0.0f)
        _chargeCooldown -= dt;

    if (_isCharging)
    {
        // 冲锋状态：沿冲锋方向快速移动
        float chargeSpeed = getSpeed() * 2.5f;
        Vec2 newPos = getPosition() + _chargeDirection * chargeSpeed * dt;
        setPosition(newPos);

        // 冲锋持续 0.4 秒
        if (_chargeCooldown <= 0.0f)
        {
            _isCharging = false;
            _chargeCooldown = 1.5f;  // 冲锋结束后冷却 1.5 秒
        }
        return;
    }
    else
    {
        // 普通状态：向玩家移动
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        // 冷却结束且玩家较近时，尝试发起冲锋
        float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
        if (_chargeCooldown <= 0.0f && distToPlayer < 150.0f)
        {
            _isCharging = true;
            _chargeDirection = dir;
            _chargeCooldown = 0.4f;  // 冲锋持续 0.4 秒
        }
    }
}

void DDLMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    // 冲锋怪物近战攻击
    _targetPlayer->takeDamage(_attackDamage);
}
