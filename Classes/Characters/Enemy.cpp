#include "Enemy.h"


USING_NS_CC;

Enemy::Enemy()
    : _targetPlayer(nullptr)
    , _attackDamage(10)
    , _attackRange(50.0f)
    , _expReward(50)
    , _hitCount(0)
    , _hitsToDie(3)
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
    _hitCount = 0;
    _hitsToDie = 3;
    _attackCooldown = 0.0f;
    hurtCooldown = 0.08f;
    hurtCooldownTimer = 0.0f;
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

int Enemy::getHitsToDieForPlayer(Player* player) const
{
    if (player == nullptr)
    {
        return 3;
    }

    switch (player->getCurrentMood())
    {
    case MoodType::Excited:
        return 2;
    case MoodType::Exhausted:
    case MoodType::Fear:
    case MoodType::Panic:
        return 4;
    case MoodType::Normal:
    case MoodType::Focus:
    case MoodType::Calm:
    case MoodType::Irritable:
    default:
        return 3;
    }
}
void Enemy::takeDamage(int damage)
{
    takeDamage(damage, DamageType::Normal, nullptr);
}

void Enemy::takeDamage(int damage, DamageType damageType, Role* attacker)
{
    if (!isAlive || !isObjectActive() || damage <= 0 || !canTakeDamage())
    {
        return;
    }

    Player* moodPlayer = dynamic_cast<Player*>(attacker);
    if (moodPlayer == nullptr)
    {
        moodPlayer = _targetPlayer;
    }

    _hitsToDie = getHitsToDieForPlayer(moodPlayer);
    ++_hitCount;
    int hitsLeft = _hitsToDie - _hitCount;
    if (hitsLeft <= 0)
    {
        hp = 0;
        die();
        return;
    }

    int newHp = (maxHp * hitsLeft + _hitsToDie - 1) / _hitsToDie;
    if (newHp < 1) newHp = 1;
    setHp(newHp);
    hurtCooldownTimer = hurtCooldown;
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

    if (hurtCooldownTimer > 0.0f)
    {
        hurtCooldownTimer -= dt;
        if (hurtCooldownTimer < 0.0f)
        {
            hurtCooldownTimer = 0.0f;
        }
    }

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
