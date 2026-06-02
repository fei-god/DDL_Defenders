#include "Player.h"

USING_NS_CC;

Player::Player()
    : baseSpeed(0.0f)
    , currentSpeed(0.0f)
    , inputDirection(Vec2::ZERO)
    , isMoving(false)
    , isInvincible(false)
    , invincibleTime(0.0f)
    , maxInvincibleTime(1.0f)
    , exp(0)
    , level(1)
{
}

Player::~Player()
{
}

bool Player::initPlayer(
    const std::string& name,
    const std::string& imagePath,
    const Vec2& startPosition,
    int maxHp,
    float baseSpeed,
    int defense
)
{
    if (!Role::initRole(
        name,
        GameObjectType::Player,
        imagePath,
        startPosition,
        maxHp,
        baseSpeed,
        defense
    ))
    {
        return false;
    }

    this->baseSpeed = baseSpeed;
    this->currentSpeed = baseSpeed;

    this->inputDirection = Vec2::ZERO;
    this->isMoving = false;

    this->isInvincible = false;
    this->invincibleTime = 0.0f;
    this->maxInvincibleTime = 1.0f;

    this->exp = 0;
    this->level = 1;

    moodSystem.initMoodSystem();
    applyMoodEffect();

    return true;
}

Player* Player::create(
    const std::string& name,
    const std::string& imagePath,
    const Vec2& startPosition,
    int maxHp,
    float baseSpeed,
    int defense
)
{
    Player* player = new Player();

    if (player && player->initPlayer(
        name,
        imagePath,
        startPosition,
        maxHp,
        baseSpeed,
        defense
    ))
    {
        player->autorelease();
        return player;
    }

    CC_SAFE_DELETE(player);
    return nullptr;
}

void Player::updatePlayer(float dt)
{
    if (!isRoleAlive() || !isObjectActive())
    {
        return;
    }

    // 更新情绪持续时间
    moodSystem.updateMood(dt);

    // 更新无敌时间
    if (isInvincible)
    {
        invincibleTime -= dt;

        if (invincibleTime <= 0.0f)
        {
            invincibleTime = 0.0f;
            isInvincible = false;
        }
    }

    applyMoodEffect();
    movePlayer(dt);
}

void Player::movePlayer(float dt)
{
    if (!isRoleAlive() || !isObjectActive())
    {
        return;
    }

    if (inputDirection == Vec2::ZERO)
    {
        isMoving = false;
        setDirection(Vec2::ZERO);
        return;
    }

    isMoving = true;

    Vec2 normalizedDirection = inputDirection.getNormalized();
    setDirection(normalizedDirection);

    Vec2 newPosition = getPosition() + normalizedDirection * currentSpeed * dt;
    setPosition(newPosition);
}

void Player::setInputDirection(const Vec2& direction)
{
    this->inputDirection = direction;
}

Vec2 Player::getInputDirection() const
{
    return inputDirection;
}

void Player::takeDamage(int damage)
{
    if (!isRoleAlive() || !isObjectActive())
    {
        return;
    }

    if (isInvincible)
    {
        return;
    }

    float damageMultiplier = moodSystem.getDamageMultiplier();
    int finalDamage = static_cast<int>(damage * damageMultiplier);

    if (finalDamage < 1)
    {
        finalDamage = 1;
    }

    Role::takeDamage(finalDamage);

    if (!isDead())
    {
        isInvincible = true;
        invincibleTime = maxInvincibleTime;
    }
}

void Player::die()
{
    Role::die();

    isMoving = false;
    inputDirection = Vec2::ZERO;

    // 这里以后可以通知 GameScene 进入 Game Over
}

void Player::changeMood(MoodType mood)
{
    moodSystem.changeMood(mood);
    applyMoodEffect();
}

void Player::changeMood(MoodType mood, float duration)
{
    moodSystem.changeMood(mood);
    moodSystem.setMoodDuration(duration);
    applyMoodEffect();
}

void Player::applyMoodEffect()
{
    currentSpeed = baseSpeed * moodSystem.getSpeedMultiplier();
    setSpeed(currentSpeed);
}

MoodType Player::getCurrentMood() const
{
    return moodSystem.getCurrentMood();
}

std::string Player::getCurrentMoodName() const
{
    return moodSystem.getMoodName();
}

float Player::getCurrentSpeed() const
{
    return currentSpeed;
}

MoodSystem* Player::getMoodSystem()
{
    return &moodSystem;
}

void Player::setInvincible(bool invincible)
{
    this->isInvincible = invincible;

    if (invincible)
    {
        invincibleTime = maxInvincibleTime;
    }
    else
    {
        invincibleTime = 0.0f;
    }
}

bool Player::getInvincible() const
{
    return isInvincible;
}

void Player::addExp(int amount)
{
    exp += amount;

    if (exp >= level * 100)
    {
        exp = 0;
        level++;
    }
}

int Player::getExp() const
{
    return exp;
}

int Player::getLevel() const
{
    return level;
}
