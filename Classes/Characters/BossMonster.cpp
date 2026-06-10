#include "BossMonster.h"
#include "cocos2d.h"

USING_NS_CC;

BossMonster* BossMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    BossMonster* boss = new BossMonster();
    if (boss && boss->initBossMonster(imagePath, startPosition, target, waveLevel))
    {
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}

bool BossMonster::initBossMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    // Boss属性：高血量，慢速度，高攻击，高防御
    // 根据波次等级调整基础属性
    int baseHp = 200 + waveLevel * 30;
    float baseSpeed = 40.0f + waveLevel * 3.0f;
    int baseDefense = 12 + waveLevel * 2;
    int baseAttack = 30 + waveLevel * 5;
    float baseRange = 70.0f;
    int baseExp = 200 + waveLevel * 50;

    bool ok = initEnemy("BossMonster",
        imagePath,
        startPosition,
        baseHp,
        baseSpeed,
        baseDefense,
        baseAttack,
        baseRange,
        baseExp);
    if (!ok) return false;

    setTargetPlayer(target);

    // 初始化Boss特有属性
    _chargeTimer = 0.0f;
    _chargeInterval = 3.0f;        // 每3秒尝试冲锋
    _isCharging = false;
    _chargeDir = Vec2::ZERO;
    _chargeDuration = 0.0f;

    _specialAttackTimer = 0.0f;
    _specialAttackInterval = 5.0f; // 每5秒特殊攻击

    _isEnraged = false;
    _waveLevel = waveLevel;

    _wanderTimer = 0.0f;
    _wanderTarget = Vec2::ZERO;
    _isWandering = true;

    // Boss体型放大1.5倍
    setScale(1.5f);

    CCLOG("BossMonster created! HP:%d Speed:%.1f Atk:%d (Wave %d)",
        getMaxHp(), getSpeed(), getAttackDamage(), waveLevel);

    return true;
}

void BossMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    // 检查是否进入狂暴状态
    if (!_isEnraged && getHp() <= getMaxHp() / 2)
    {
        _isEnraged = true;
        // 狂暴：速度提升50%，攻击间隔缩短
        setSpeed(getSpeed() * 1.5f);
        _chargeInterval = 2.0f;
        _specialAttackInterval = 3.0f;
        CCLOG("BossMonster ENRAGED! Speed:%.1f", getSpeed());
    }

    // 更新技能计时器
    if (_chargeTimer > 0.0f)
        _chargeTimer -= dt;
    if (_specialAttackTimer > 0.0f)
        _specialAttackTimer -= dt;

    if (_isCharging)
    {
        // 冲锋状态
        _chargeDuration -= dt;
        float chargeSpeed = getSpeed() * 3.0f;
        Vec2 newPos = getPosition() + _chargeDir * chargeSpeed * dt;
        setPosition(newPos);

        if (_chargeDuration <= 0.0f)
        {
            _isCharging = false;
            _isWandering = true;
            _wanderTimer = 1.0f; // 冲锋后短暂徘徊
        }
        return;
    }

    // 特殊攻击：在攻击范围内释放AOE
    float distToPlayer = getPosition().distance(_targetPlayer->getPosition());
    if (_specialAttackTimer <= 0.0f && distToPlayer <= _attackRange * 1.5f)
    {
        performSpecialAttack();
        _specialAttackTimer = _specialAttackInterval;
        return;
    }

    // 冲锋判定：冷却完毕且玩家在中等距离
    if (_chargeTimer <= 0.0f && distToPlayer > 60.0f && distToPlayer < 300.0f)
    {
        startCharge();
        return;
    }

    // 正常移动：缓慢接近玩家 + 小幅度徘徊
    if (_isWandering)
    {
        _wanderTimer -= dt;
        if (_wanderTimer <= 0.0f || _wanderTarget == Vec2::ZERO)
        {
            // 生成新的徘徊目标（玩家周围随机偏移）
            float offsetX = (CCRANDOM_0_1() - 0.5f) * 200.0f;
            float offsetY = (CCRANDOM_0_1() - 0.5f) * 200.0f;
            _wanderTarget = _targetPlayer->getPosition() + Vec2(offsetX, offsetY);
            _wanderTimer = 2.0f + CCRANDOM_0_1() * 2.0f;
        }

        // 移向徘徊目标
        Vec2 dir = _wanderTarget - getPosition();
        float wanderDist = dir.length();
        if (wanderDist < 20.0f)
        {
            _isWandering = false;
        }
        else
        {
            dir.normalize();
            setDirection(dir);
            Vec2 newPos = getPosition() + dir * getSpeed() * dt;
            setPosition(newPos);
        }
    }
    else
    {
        // 直接追踪玩家
        Vec2 dir = _targetPlayer->getPosition() - getPosition();
        dir.normalize();
        setDirection(dir);
        Vec2 newPos = getPosition() + dir * getSpeed() * dt;
        setPosition(newPos);

        // 2-3秒后恢复徘徊
        _wanderTimer -= dt;
        if (_wanderTimer <= 0.0f)
        {
            _isWandering = true;
            _wanderTimer = 0.0f;
        }
    }
}

void BossMonster::startCharge()
{
    _isCharging = true;
    _isWandering = false;
    _chargeDir = _targetPlayer->getPosition() - getPosition();
    _chargeDir.normalize();
    _chargeDuration = 0.6f;  // 冲锋持续0.6秒
    _chargeTimer = _chargeInterval;

    CCLOG("BossMonster starts charging!");
}

void BossMonster::performSpecialAttack()
{
    // Boss范围攻击：对玩家造成额外伤害
    // 实际伤害计算在attack()中，这里只是触发布局
    CCLOG("BossMonster performs special AOE attack!");

    // 对玩家造成双倍伤害（通过临时提升攻击力）
    int originalDamage = getAttackDamage();
    setAttackDamage(originalDamage * 2);
    attack();
    setAttackDamage(originalDamage);
}

void BossMonster::attack()
{
    if (_targetPlayer == nullptr) return;

    // Boss攻击：造成基础伤害
    _targetPlayer->takeDamage(_attackDamage);

    // Boss攻击带有短暂击退效果（如果玩家类支持的话，通过修改位置模拟）
    // 注：此功能可选，可根据需要启用
    /*
    Vec2 knockDir = _targetPlayer->getPosition() - getPosition();
    knockDir.normalize();
    Vec2 knockPos = _targetPlayer->getPosition() + knockDir * 30.0f;
    _targetPlayer->setPosition(knockPos);
    */
}

void BossMonster::die()
{
    if (!isRoleAlive()) return;

    // Boss死亡：大量经验奖励
    if (_targetPlayer != nullptr && _targetPlayer->isRoleAlive())
    {
        _targetPlayer->addExp(_expReward);
        CCLOG("Boss defeated! Player gained %d exp!", _expReward);
    }

    // 创建死亡特效（缩放缩小消失）
    auto fadeOut = ScaleTo::create(0.3f, 0.1f);
    auto remove = RemoveSelf::create();
    this->runAction(Sequence::create(fadeOut, remove, nullptr));

    Role::die();
}

bool BossMonster::isEnraged() const
{
    return _isEnraged;
}
