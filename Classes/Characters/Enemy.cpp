#include "Enemy.h"


USING_NS_CC;

Enemy::Enemy()
    : _targetPlayer(nullptr)
    , _attackDamage(10)
    , _attackRange(50.0f)
    , _expReward(50)
    , _attackCooldown(0.0f)
    , _attackCooldownMax(1.0f)
{
}

Enemy::~Enemy()
{
}

bool Enemy::initEnemy(const std::string& name,
    const std::string& imagePath,
    const cocos2d::Vec2& startPosition,
    int maxHp,
    float speed,
    int defense,
    int attackDamage,
    float attackRange,
    int expReward)
{
    if (!Role::initRole(name, GameObjectType::Enemy, imagePath, startPosition, maxHp, speed, defense))
        return false;

    _attackDamage = attackDamage;
    _attackRange = attackRange;
    _expReward = expReward;
    _attackCooldown = 0.0f;
    return true;
}

void Enemy::setTargetPlayer(Player* player)
{
    _targetPlayer = player;
}

Player* Enemy::getTargetPlayer() const
{
    return _targetPlayer;
}

void Enemy::setAttackDamage(int damage)
{
    _attackDamage = damage;
}

int Enemy::getAttackDamage() const
{
    return _attackDamage;
}

void Enemy::setAttackRange(float range)
{
    _attackRange = range;
}

float Enemy::getAttackRange() const
{
    return _attackRange;
}

void Enemy::setExpReward(int exp)
{
    _expReward = exp;
}

int Enemy::getExpReward() const
{
    return _expReward;
}

void Enemy::die()
{
    if (!isRoleAlive()) return;

    // 给玩家增加经验
    if (_targetPlayer != nullptr && _targetPlayer->isRoleAlive())
    {
        _targetPlayer->addExp(_expReward);
    }

    Role::die();  // 调用基类死亡逻辑（设置 isAlive=false，停止激活等）
}

void Enemy::updateEnemy(float dt)
{
    if (!isRoleAlive() || !isObjectActive())
        return;

    // 更新攻击冷却
    if (_attackCooldown > 0.0f)
        _attackCooldown -= dt;

    // 调用派生类实现的移动逻辑
    move(dt);

    // 自动检测与玩家的距离，如果在攻击范围内且冷却就绪则攻击
    if (_targetPlayer != nullptr && _targetPlayer->isRoleAlive())
    {
        float dist = getPosition().distance(_targetPlayer->getPosition());
        if (dist <= _attackRange && _attackCooldown <= 0.0f)
        {
            attack();
            _attackCooldown = _attackCooldownMax;
        }
    }
}