#include "Weapon.h"
#include <cfloat>
#include <cmath>
#include <new>
#include <algorithm>

USING_NS_CC;

namespace
{
    void faceWeaponWithoutUpsideDown(Weapon* weapon, const Vec2& direction)
    {
        if (weapon == nullptr || direction.isZero())
        {
            return;
        }

        Vec2 normalized = direction.getNormalized();
        float angle = CC_RADIANS_TO_DEGREES(std::atan2(normalized.y, std::abs(normalized.x)));

        weapon->setDirection(normalized);
        weapon->setFlippedX(normalized.x < 0.0f);
        weapon->setFlippedY(false);
        weapon->setRotation(-angle);
    }
}

Weapon::Weapon()
    : _owner(nullptr)
    , _enemyList(nullptr)
    , _bulletList(nullptr)
    , _bulletLayer(nullptr)
    , _bulletPool(nullptr)
    , _attackPower(0)
    , _attackRange(500.0f)
    , _cooldownTime(1.0f)
    , _cooldownTimer(0.0f)
    , _maxEnergy(100.0f)
    , _currentEnergy(100.0f)
    , _energyCost(20.0f)
    , _energyRecoverPerSecond(18.0f)
    , _projectileCountBonus(0)
    , _bulletSpeed(500.0f)
    , _bulletImagePath("")
    , _aimDirection(Vec2(1, 0))
{
}

void Weapon::bindBulletPool(BulletPool* bulletPool)
{
    _bulletPool = bulletPool;
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
    _maxEnergy = 100.0f;
    _currentEnergy = _maxEnergy;
    _energyCost = 20.0f;
    _energyRecoverPerSecond = 18.0f;
    _projectileCountBonus = 0;

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
        Vec2 displayDir = _aimDirection.lengthSquared() > 0.0001f
            ? _aimDirection.getNormalized()
            : Vec2(1, 0);
        Vec2 side(-displayDir.y, displayDir.x);
        Vec2 displayOffset = displayDir * 20.0f + side * 4.0f + Vec2(0.0f, -8.0f);
        if (getParent() == _owner)
        {
            setObjectPosition(displayOffset);
        }
        else
        {
            setObjectPosition(_owner->getObjectPosition() + displayOffset);
        }
        faceWeaponWithoutUpsideDown(this, displayDir);
    }

    // 冷却时间每帧减少 dt。
    if (_cooldownTimer > 0.0f)
    {
        _cooldownTimer -= dt;
    }

    if (_currentEnergy < _maxEnergy)
    {
        _currentEnergy += _energyRecoverPerSecond * dt;
        if (_currentEnergy > _maxEnergy)
        {
            _currentEnergy = _maxEnergy;
        }
    }

    if (canFire())
    {
        fire();
    }
}

void Weapon::updateCooldown(float dt)
{
    if (_owner != nullptr)
    {
        Vec2 displayDir = _aimDirection.lengthSquared() > 0.0001f
            ? _aimDirection.getNormalized()
            : Vec2(1, 0);
        Vec2 side(-displayDir.y, displayDir.x);
        Vec2 displayOffset = displayDir * 20.0f + side * 4.0f + Vec2(0.0f, -8.0f);
        if (getParent() == _owner)
        {
            setObjectPosition(displayOffset);
        }
        else
        {
            setObjectPosition(_owner->getObjectPosition() + displayOffset);
        }
        faceWeaponWithoutUpsideDown(this, displayDir);
    }

    if (_cooldownTimer > 0.0f)
    {
        _cooldownTimer -= dt;
        if (_cooldownTimer < 0.0f)
        {
            _cooldownTimer = 0.0f;
        }
    }

    if (_currentEnergy < _maxEnergy)
    {
        _currentEnergy += _energyRecoverPerSecond * dt;
        if (_currentEnergy > _maxEnergy)
        {
            _currentEnergy = _maxEnergy;
        }
    }
}

void Weapon::readyNow()
{
    _cooldownTimer = 0.0f;
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

    if (!hasEnoughEnergy())
    {
        return false;
    }

    if (_enemyList == nullptr || _bulletList == nullptr || _bulletLayer == nullptr)
    {
        return false;
    }

    return true;
}

void Weapon::configureEnergy(float maxEnergy, float energyCost, float recoverPerSecond)
{
    _maxEnergy = maxEnergy > 1.0f ? maxEnergy : 1.0f;
    _currentEnergy = _maxEnergy;
    _energyCost = energyCost > 0.0f ? energyCost : 1.0f;
    _energyRecoverPerSecond = recoverPerSecond > 0.0f ? recoverPerSecond : 1.0f;
}

bool Weapon::isReadyToFire() const
{
    return canFire();
}

bool Weapon::hasEnoughEnergy() const
{
    return _currentEnergy >= _energyCost;
}

float Weapon::getEnergyRatio() const
{
    if (_maxEnergy <= 0.0f)
    {
        return 0.0f;
    }

    float ratio = _currentEnergy / _maxEnergy;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    return ratio;
}

float Weapon::getCurrentEnergy() const
{
    return _currentEnergy;
}

float Weapon::getMaxEnergy() const
{
    return _maxEnergy;
}

float Weapon::getEnergyCost() const
{
    return _energyCost;
}

float Weapon::getEnergyRecoverPerSecond() const
{
    return _energyRecoverPerSecond;
}

bool Weapon::consumeEnergyForShot()
{
    if (!hasEnoughEnergy())
    {
        return false;
    }

    _currentEnergy -= _energyCost;
    if (_currentEnergy < 0.0f)
    {
        _currentEnergy = 0.0f;
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

void Weapon::setAimDirection(const Vec2& direction)
{
    if (direction.lengthSquared() > 0.0001f)
    {
        _aimDirection = direction.getNormalized();
    }
}

Vec2 Weapon::getAimDirection() const
{
    return _aimDirection;
}

std::string Weapon::getWeaponName() const
{
    return getObjectName();
}

int Weapon::getModifiedAttackPower() const
{
    float multiplier = 1.0f;
    if (_owner != nullptr && _owner->getMoodSystem() != nullptr)
    {
        multiplier = _owner->getMoodSystem()->getOutgoingDamageMultiplier();
    }

    int damage = static_cast<int>(_attackPower * multiplier + 0.5f);
    return damage < 1 ? 1 : damage;
}

int Weapon::getAttackPower() const
{
    return _attackPower;
}

float Weapon::getCooldownTime() const
{
    return _cooldownTime;
}

int Weapon::getProjectileCountBonus() const
{
    return _projectileCountBonus;
}

float Weapon::getAttackRange() const
{
    return _attackRange;
}

void Weapon::setAttackRange(float range)
{
    _attackRange = range;
}

void Weapon::addAttackPower(int amount)
{
    _attackPower = std::max(1, _attackPower + amount);
}

void Weapon::addEnergyRecoverPercent(float percent)
{
    _energyRecoverPerSecond *= (1.0f + percent);
    if (_energyRecoverPerSecond < 1.0f)
    {
        _energyRecoverPerSecond = 1.0f;
    }
}

void Weapon::addMaxEnergy(float amount)
{
    float oldMax = _maxEnergy;
    _maxEnergy = std::max(1.0f, _maxEnergy + amount);
    _currentEnergy += (_maxEnergy - oldMax);
    if (_currentEnergy > _maxEnergy)
    {
        _currentEnergy = _maxEnergy;
    }
}

void Weapon::addProjectileCountBonus(int amount)
{
    _projectileCountBonus = std::max(0, _projectileCountBonus + amount);
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

Vec2 Weapon::getMuzzlePosition(const Vec2& direction) const
{
    if (_owner == nullptr)
    {
        return getPosition();
    }

    Vec2 shotDir = direction.lengthSquared() > 0.0001f
        ? direction.getNormalized()
        : getAimDirection();
    if (shotDir.lengthSquared() < 0.0001f)
    {
        shotDir = Vec2(1, 0);
    }

    Vec2 localMuzzle = getPosition() + shotDir * 26.0f;
    Vec2 worldMuzzle = getParent()
        ? getParent()->convertToWorldSpace(localMuzzle)
        : _owner->getObjectPosition() + shotDir * 46.0f;

    if (_bulletLayer)
    {
        return _bulletLayer->convertToNodeSpace(worldMuzzle);
    }

    return worldMuzzle;
}

Bullet* Weapon::spawnBullet(const std::string& name,
    const std::string& imagePath,
    const Vec2& startPosition,
    const Vec2& direction,
    float speed,
    int damage,
    float lifeTime,
    bool canPierce)
{
    Bullet* bullet = nullptr;
    if (_bulletPool != nullptr)
    {
        bullet = _bulletPool->acquire(name, imagePath, startPosition, direction,
            speed, damage, lifeTime, canPierce);
    }
    else
    {
        bullet = Bullet::createBullet(name, imagePath, startPosition, direction,
            speed, damage, lifeTime, canPierce);
        if (bullet && _bulletLayer)
        {
            _bulletLayer->addChild(bullet);
        }
    }

    if (bullet && _bulletList)
    {
        _bulletList->push_back(bullet);
    }

    return bullet;
}
