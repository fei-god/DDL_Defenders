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
    // 冲刺怪属性：高攻高速但血量稍低
    bool ok = initEnemy("DDLMonster",
        imagePath,
        startPosition,
        60,        // maxHp
        200.0f,    // speed (快)
        10,        // defense
        35,        // attackDamage
        60.0f,     // attackRange
        80);       // expReward
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

    // 冲刺冷却更新
    if (_chargeCooldown > 0.0f)
        _chargeCooldown -= dt;

    if (_isCharging)
    {
        // 冲刺状态：高速直线移动一段时间
        float chargeSpeed = getSpeed() * 2.5f;
        Vec2 newPos = getPosition() + _chargeDirection * chargeSpeed * dt;
        setPosition(newPos);
        // 简单冲刺持续时间 0.4 秒后结束
        if (_chargeCooldown <= 0.0f)
        {
            _isCharging = false;
        }
        return;
    }
    else
    {
        // 普通状态：朝玩家移动
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        // 每隔 1.5 秒尝试发动冲刺（距离玩家较近时）
        if (_chargeCooldown <= 0.0f && getPosition().distance(_targetPlayer->getPosition()) < 150.0f)
        {
            _isCharging = true;
            _chargeDirection = dir;
            _chargeCooldown = 0.4f;   // 冲刺持续 0.4 秒
            // 额外再设置一个冷却，防止连续冲刺
            _chargeCooldown = 1.5f;
        }
    }
}

void DDLMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    // 冲刺怪攻击力高
    _targetPlayer->takeDamage(_attackDamage);
}