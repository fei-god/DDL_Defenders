#include "Weapon.h"
#include <cfloat>
#include <new>

USING_NS_CC;

Weapon::Weapon()
    : _owner(nullptr)
    , _enemyList(nullptr)
    , _bulletList(nullptr)
    , _bulletLayer(nullptr)
    , _attackPower(0)
    , _cooldownTime(1.0f)
    , _cooldownTimer(0.0f)
    , _bulletSpeed(500.0f)
    , _bulletImagePath("")
{
}

Weapon::~Weapon()
{
}

bool Weapon::initWeapon(
    const string& name,
    const string& imagePath,
    Player* owner,
    int attackPower,
    float cooldownTime
)
{

    if (!GameObject::initObject(
        name,
        GameObjectType::Weapon,
        imagePath,
        owner ? owner->getObjectPosition() : Vec2::ZERO
    ))
    {
        return false;
    }

    _owner = owner;
    _attackPower = attackPower;
    _cooldownTime = cooldownTime;
    _cooldownTimer = 0.0f;

    return true;
}

void Weapon::bindBattleData(
    vector<Enemy*>* enemyList,
    vector<Bullet*>* bulletList,
    Node* bulletLayer
)
{
    _enemyList = enemyList;
    _bulletList = bulletList;
    _bulletLayer = bulletLayer;
}

void Weapon::updateObject(float dt)
{
    if (!isObjectActive())
    {
        return;
    }

    // 武器跟随玩家位置。
    if (_owner != nullptr)
    {
        setObjectPosition(_owner->getObjectPosition());
    }

    // 冷却时间每帧减少 dt。
    if (_cooldownTimer > 0.0f)
    {
        _cooldownTimer -= dt;
    }

    if (canFire())
    {
        fire();
    }
}

bool Weapon::canFire() const
{
    if (_cooldownTimer > 0.0f)
    {
        return false;
    }

    if (_owner == nullptr || !_owner->isRoleAlive())
    {
        return false;
    }

    if (_enemyList == nullptr || _bulletList == nullptr || _bulletLayer == nullptr)
    {
        return false;
    }

    return true;
}

void Weapon::resetCooldown()
{
    float multiplier = 1.0f;

    // Focus 会减少冷却，Exhausted 会增加冷却。
    if (_owner != nullptr && _owner->getMoodSystem() != nullptr)
    {
        multiplier = _owner->getMoodSystem()->getCooldownMultiplier();
    }

    _cooldownTimer = _cooldownTime * multiplier;
}

int Weapon::getAttackPower() const
{
    return _attackPower;
}

float Weapon::getCooldownTime() const
{
    return _cooldownTime;
}

Enemy* Weapon::findNearestEnemy() const
{
    if (_owner == nullptr || _enemyList == nullptr)
    {
        return nullptr;
    }

    Enemy* nearestEnemy = nullptr;
    float nearestDistanceSq = FLT_MAX;

    Vec2 playerPos = _owner->getObjectPosition();

    for (Enemy* enemy : *_enemyList)
    {
        if (enemy == nullptr)
        {
            continue;
        }

        if (!enemy->isObjectActive() || !enemy->isRoleAlive())
        {
            continue;
        }

        Vec2 enemyPos = enemy->getObjectPosition();

        float distanceSq = playerPos.distanceSquared(enemyPos);

        if (distanceSq < nearestDistanceSq)
        {
            nearestDistanceSq = distanceSq;
            nearestEnemy = enemy;
        }
    }

    return nearestEnemy;
}

Vec2 Weapon::getDirectionToEnemy(Enemy* enemy) const
{
    if (_owner == nullptr || enemy == nullptr)
    {
        return Vec2(1, 0);
    }

    Vec2 direction = enemy->getObjectPosition() - _owner->getObjectPosition();

    if (direction.lengthSquared() < 0.0001f)
    {
        return Vec2(1, 0);
    }

    return direction.getNormalized();
}