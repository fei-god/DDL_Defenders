#include "Player.h"

#include <algorithm>
#include <sstream>
#include <cmath>

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
    , expToNextLevel(100)
    , upgradePoints(0)
    , material(0)
    , gold(0)
    , harvesting(0)
    , luck(0.0f)
    , pickupRange(80.0f)
    , hpRegen(0.0f)
    , enemySpeedModifier(0.0f)
    , maxWeaponCount(6)
    , currentTarget(nullptr)
    , hpRegenTimer(0.0f)
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

    this->baseSpeed = std::max(0.0f, baseSpeed);
    this->currentSpeed = this->baseSpeed;

    this->inputDirection = Vec2::ZERO;
    this->isMoving = false;

    this->isInvincible = false;
    this->invincibleTime = 0.0f;
    this->maxInvincibleTime = 1.0f;

    this->exp = 0;
    this->level = 1;
    this->expToNextLevel = 100;
    this->upgradePoints = 0;

    this->material = 0;
    this->gold = 0;

    this->harvesting = 0;
    this->luck = 0.0f;
    this->pickupRange = 80.0f;
    this->hpRegen = 0.0f;
    this->enemySpeedModifier = 0.0f;

    this->maxWeaponCount = 6;
    this->weapons.clear();
    this->currentTarget = nullptr;

    this->hpRegenTimer = 0.0f;

    setObjectCamp(GameObjectCamp::Player);
    addTag("Player");
    addTag("Role");

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

void Player::updateObject(float dt)
{
    updatePlayer(dt);
}

void Player::updatePlayer(float dt)
{
    if (!isRoleAlive() || !isObjectActive())
    {
        return;
    }

    // GameObject 的生命周期更新
    updateLifeTime(dt);

    if (isExpired())
    {
        markForDestroy();
        return;
    }

    // Role 的无敌、受伤冷却、状态效果更新
    if (invincibleTimer > 0.0f)
    {
        invincibleTimer -= dt;
        if (invincibleTimer < 0.0f)
        {
            invincibleTimer = 0.0f;
        }
    }

    if (hurtCooldownTimer > 0.0f)
    {
        hurtCooldownTimer -= dt;
        if (hurtCooldownTimer < 0.0f)
        {
            hurtCooldownTimer = 0.0f;
        }
    }

    updateStatusEffects(dt);

    // 玩家自己的无敌时间
    if (isInvincible)
    {
        invincibleTime -= dt;

        if (invincibleTime <= 0.0f)
        {
            invincibleTime = 0.0f;
            isInvincible = false;
        }
    }

    // 更新情绪
    moodSystem.updateMood(dt);
    applyMoodEffect();

    // 生命恢复：每 1 秒判定一次
    if (hpRegen > 0.0f)
    {
        hpRegenTimer += dt;

        if (hpRegenTimer >= 1.0f)
        {
            hpRegenTimer = 0.0f;

            int healAmount = static_cast<int>(std::floor(hpRegen));

            if (healAmount < 1)
            {
                healAmount = 1;
            }

            heal(healAmount);
        }
    }

    movePlayer(dt);
}

// =========================
// 移动输入
// =========================

void Player::movePlayer(float dt)
{
    if (!isRoleAlive() || !isObjectActive())
    {
        return;
    }

    if (isStunned())
    {
        isMoving = false;
        setDirection(Vec2::ZERO);
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

    Vec2 newPosition = getPosition() + normalizedDirection * getCurrentSpeed() * dt;
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

bool Player::getIsMoving() const
{
    return isMoving;
}

// =========================
// 玩家受伤 / 死亡
// =========================

void Player::takeDamage(int damage)
{
    takeDamage(damage, DamageType::Normal, nullptr);
}

void Player::takeDamage(int damage, DamageType damageType, Role* attacker)
{
    if (!isRoleAlive() || !isObjectActive())
    {
        return;
    }

    if (isInvincible || isInvincibleNow())
    {
        return;
    }

    float damageMultiplier = moodSystem.getDamageMultiplier();

    int moodDamage = static_cast<int>(std::ceil(damage * damageMultiplier));

    if (moodDamage < 1)
    {
        moodDamage = 1;
    }

    Role::takeDamage(moodDamage, damageType, attacker);

    if (!isDead())
    {
        isInvincible = true;
        invincibleTime = maxInvincibleTime;
        setInvincible(maxInvincibleTime);
    }
}

void Player::die()
{
    Role::die();

    isMoving = false;
    inputDirection = Vec2::ZERO;
    currentTarget = nullptr;

    // 后面 GameScene 可以检测 player->isDead() 进入 Game Over
}

// =========================
// 情绪系统
// =========================

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

    if (currentSpeed < 0.0f)
    {
        currentSpeed = 0.0f;
    }

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

MoodSystem* Player::getMoodSystem()
{
    return &moodSystem;
}

// =========================
// 速度
// =========================

float Player::getBaseSpeed() const
{
    return baseSpeed;
}

void Player::setBaseSpeed(float speed)
{
    this->baseSpeed = std::max(0.0f, speed);
    applyMoodEffect();
}

float Player::getCurrentSpeed() const
{
    return Role::getCurrentSpeed();
}

// =========================
// 无敌状态
// =========================

void Player::setInvincible(bool invincible)
{
    this->isInvincible = invincible;

    if (invincible)
    {
        invincibleTime = maxInvincibleTime;
        Role::setInvincible(maxInvincibleTime);
    }
    else
    {
        invincibleTime = 0.0f;
    }
}

void Player::setInvincibleTime(float time)
{
    maxInvincibleTime = std::max(0.0f, time);
}

bool Player::getInvincible() const
{
    return isInvincible || isInvincibleNow();
}

float Player::getInvincibleTime() const
{
    return invincibleTime;
}

// =========================
// 经验和等级
// =========================

void Player::addExp(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    exp += amount;

    while (canLevelUp())
    {
        levelUp();
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

void Player::setLevel(int level)
{
    this->level = std::max(1, level);
    this->expToNextLevel = 100 + (this->level - 1) * 50;
}

int Player::getExpToNextLevel() const
{
    return expToNextLevel;
}

float Player::getExpPercent() const
{
    if (expToNextLevel <= 0)
    {
        return 0.0f;
    }

    return static_cast<float>(exp) / static_cast<float>(expToNextLevel);
}

bool Player::canLevelUp() const
{
    return exp >= expToNextLevel;
}

void Player::levelUp()
{
    if (expToNextLevel <= 0)
    {
        return;
    }

    exp -= expToNextLevel;
    level++;

    upgradePoints++;

    // 越到后期升级越慢
    expToNextLevel = 100 + (level - 1) * 50;

    // 升级时给一点基础成长，避免玩家完全不成长
    addMaxHp(1);
}

int Player::getUpgradePoints() const
{
    return upgradePoints;
}

void Player::addUpgradePoint(int amount)
{
    upgradePoints += amount;

    if (upgradePoints < 0)
    {
        upgradePoints = 0;
    }
}

bool Player::spendUpgradePoint(int amount)
{
    if (amount <= 0)
    {
        return false;
    }

    if (upgradePoints < amount)
    {
        return false;
    }

    upgradePoints -= amount;
    return true;
}

// =========================
// 材料 / 金币
// =========================

void Player::addMaterial(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    material += amount;
    gold += amount;
}

bool Player::spendMaterial(int amount)
{
    if (amount <= 0)
    {
        return false;
    }

    if (material < amount)
    {
        return false;
    }

    material -= amount;

    if (gold >= amount)
    {
        gold -= amount;
    }

    return true;
}

int Player::getMaterial() const
{
    return material;
}

void Player::setMaterial(int amount)
{
    material = std::max(0, amount);
    gold = material;
}

void Player::addGold(int amount)
{
    addMaterial(amount);
}

bool Player::spendGold(int amount)
{
    return spendMaterial(amount);
}

int Player::getGold() const
{
    return gold;
}

// =========================
// 玩家经济 / 成长属性
// =========================

int Player::getHarvesting() const
{
    return harvesting;
}

void Player::setHarvesting(int value)
{
    harvesting = std::max(0, value);
}

void Player::addHarvesting(int value)
{
    setHarvesting(harvesting + value);
}

float Player::getLuck() const
{
    return luck;
}

void Player::setLuck(float value)
{
    luck = value;
}

void Player::addLuck(float value)
{
    luck += value;
}

float Player::getPickupRange() const
{
    return pickupRange;
}

void Player::setPickupRange(float range)
{
    pickupRange = std::max(0.0f, range);
}

void Player::addPickupRange(float value)
{
    setPickupRange(pickupRange + value);
}

float Player::getHpRegen() const
{
    return hpRegen;
}

void Player::setHpRegen(float value)
{
    hpRegen = std::max(0.0f, value);
}

void Player::addHpRegen(float value)
{
    setHpRegen(hpRegen + value);
}

float Player::getEnemySpeedModifier() const
{
    return enemySpeedModifier;
}

void Player::setEnemySpeedModifier(float value)
{
    enemySpeedModifier = value;
}

// =========================
// 武器槽
// =========================

int Player::getMaxWeaponCount() const
{
    return maxWeaponCount;
}

void Player::setMaxWeaponCount(int count)
{
    maxWeaponCount = std::max(0, count);

    while (static_cast<int>(weapons.size()) > maxWeaponCount)
    {
        weapons.pop_back();
    }
}

bool Player::canAddWeapon() const
{
    return static_cast<int>(weapons.size()) < maxWeaponCount;
}

bool Player::addWeapon(GameObject* weapon)
{
    if (weapon == nullptr)
    {
        return false;
    }

    if (!canAddWeapon())
    {
        return false;
    }

    auto iter = std::find(weapons.begin(), weapons.end(), weapon);

    if (iter != weapons.end())
    {
        return false;
    }

    weapons.push_back(weapon);
    return true;
}

bool Player::removeWeapon(GameObject* weapon)
{
    auto iter = std::find(weapons.begin(), weapons.end(), weapon);

    if (iter == weapons.end())
    {
        return false;
    }

    weapons.erase(iter);
    return true;
}

void Player::clearWeapons()
{
    weapons.clear();
}

const std::vector<GameObject*>& Player::getWeapons() const
{
    return weapons;
}

int Player::getWeaponCount() const
{
    return static_cast<int>(weapons.size());
}

// =========================
// 自动瞄准 / 目标
// =========================

void Player::setTarget(GameObject* target)
{
    currentTarget = target;
}

GameObject* Player::getTarget() const
{
    return currentTarget;
}

bool Player::hasTarget() const
{
    return currentTarget != nullptr &&
        currentTarget->isObjectActive() &&
        !currentTarget->isMarkedForDestroy();
}

void Player::clearTarget()
{
    currentTarget = nullptr;
}

GameObject* Player::findNearestTarget(const std::vector<GameObject*>& candidates, float searchRange) const
{
    GameObject* nearest = nullptr;
    float nearestDistance = searchRange;

    for (auto object : candidates)
    {
        if (object == nullptr)
        {
            continue;
        }

        if (!object->isObjectActive() || object->isMarkedForDestroy())
        {
            continue;
        }

        if (!isEnemyCampWith(object))
        {
            continue;
        }

        float distance = distanceTo(object);

        if (distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearest = object;
        }
    }

    return nearest;
}

// =========================
// 拾取与交互
// =========================

bool Player::canPickup(GameObject* item) const
{
    if (item == nullptr)
    {
        return false;
    }

    if (!item->isObjectActive() || item->isMarkedForDestroy())
    {
        return false;
    }

    if (item->getObjectType() != GameObjectType::Item)
    {
        return false;
    }

    return isItemInPickupRange(item);
}

bool Player::isItemInPickupRange(GameObject* item) const
{
    if (item == nullptr)
    {
        return false;
    }

    return distanceTo(item) <= pickupRange;
}

void Player::pickupExp(int amount)
{
    addExp(amount);
}

void Player::pickupMaterial(int amount)
{
    addMaterial(amount);
}

void Player::pickupHeal(int amount)
{
    heal(amount);
}

// =========================
// 波次开始 / 结束
// =========================

void Player::onWaveStart()
{
    clearTarget();
    inputDirection = Vec2::ZERO;
    isMoving = false;
}

void Player::onWaveEnd()
{
    clearTarget();

    // Brotato 风格：波次结束时，收获属性转化为材料
    if (harvesting > 0)
    {
        addMaterial(harvesting);
    }
}

// =========================
// 玩家属性总览
// =========================

std::string Player::getPlayerStatsInfo() const
{
    std::ostringstream oss;

    oss << "Level: " << level
        << " | EXP: " << exp << "/" << expToNextLevel
        << " | HP: " << getHp() << "/" << getMaxHp()
        << " | Material: " << material
        << " | Weapons: " << getWeaponCount() << "/" << maxWeaponCount
        << " | Speed: " << getCurrentSpeed()
        << " | Damage: " << getBaseDamage()
        << " | Melee: " << getMeleeDamage()
        << " | Ranged: " << getRangedDamage()
        << " | Elemental: " << getElementalDamage()
        << " | Armor: " << getArmor()
        << " | Dodge: " << getDodgeChance()
        << " | Crit: " << getCritChance()
        << " | Luck: " << luck
        << " | Harvesting: " << harvesting
        << " | Mood: " << getCurrentMoodName();

    return oss.str();
}

std::string Player::getDebugInfo() const
{
    std::ostringstream oss;

    oss << "[Player]"
        << " id=" << getObjectId()
        << " name=" << getObjectName()
        << " level=" << level
        << " exp=" << exp << "/" << expToNextLevel
        << " hp=" << getHp() << "/" << getMaxHp()
        << " material=" << material
        << " speed=" << getCurrentSpeed()
        << " weapons=" << getWeaponCount() << "/" << maxWeaponCount
        << " mood=" << getCurrentMoodName()
        << " moving=" << (isMoving ? "true" : "false")
        << " invincible=" << (getInvincible() ? "true" : "false");

    return oss.str();
}
