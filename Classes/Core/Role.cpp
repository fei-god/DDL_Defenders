#include "Role.h"

USING_NS_CC;

Role::Role()
    : hp(0)
    , maxHp(0)
    , speed(0.0f)
    , direction(Vec2::ZERO)
    , isAlive(true)
    , defense(0)
{
}

Role::~Role()
{
}

bool Role::initRole(
    const std::string& name,
    GameObjectType type,
    const std::string& imagePath,
    const Vec2& startPosition,
    int maxHp,
    float speed,
    int defense
)
{
    if (!GameObject::initObject(name, type, imagePath, startPosition))
    {
        return false;
    }

    this->maxHp = maxHp;
    this->hp = maxHp;
    this->speed = speed;
    this->defense = defense;
    this->direction = Vec2::ZERO;
    this->isAlive = true;

    return true;
}

void Role::move(float dt)
{
    if (!isAlive || !isObjectActive())
    {
        return;
    }

    if (direction == Vec2::ZERO)
    {
        return;
    }

    Vec2 normalizedDirection = direction.getNormalized();
    Vec2 newPosition = getPosition() + normalizedDirection * speed * dt;

    setPosition(newPosition);
}

void Role::setDirection(const Vec2& direction)
{
    this->direction = direction;
}

Vec2 Role::getDirection() const
{
    return direction;
}

void Role::takeDamage(int damage)
{
    if (!isAlive || !isObjectActive())
    {
        return;
    }

    int finalDamage = damage - defense;

    if (finalDamage < 1)
    {
        finalDamage = 1;
    }

    hp -= finalDamage;

    if (hp <= 0)
    {
        hp = 0;
        die();
    }
}

void Role::heal(int amount)
{
    if (!isAlive || !isObjectActive())
    {
        return;
    }

    hp += amount;

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

bool Role::isDead() const
{
    return !isAlive || hp <= 0;
}

void Role::die()
{
    isAlive = false;
    hp = 0;
    direction = Vec2::ZERO;

    setActive(false);
}

int Role::getHp() const
{
    return hp;
}

void Role::setHp(int hp)
{
    this->hp = hp;

    if (this->hp > maxHp)
    {
        this->hp = maxHp;
    }

    if (this->hp <= 0)
    {
        this->hp = 0;
        die();
    }
}

int Role::getMaxHp() const
{
    return maxHp;
}

void Role::setMaxHp(int maxHp)
{
    this->maxHp = maxHp;

    if (hp > this->maxHp)
    {
        hp = this->maxHp;
    }
}

float Role::getSpeed() const
{
    return speed;
}

void Role::setSpeed(float speed)
{
    this->speed = speed;
}

int Role::getDefense() const
{
    return defense;
}

void Role::setDefense(int defense)
{
    this->defense = defense;
}

bool Role::isRoleAlive() const
{
    return isAlive;
}
