#include "SleepyMonster.h"
#include "cocos2d.h"

USING_NS_CC;

SleepyMonster* SleepyMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    SleepyMonster* monster = new SleepyMonster();
    if (monster && monster->initSleepyMonster(imagePath, startPosition, target))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool SleepyMonster::initSleepyMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target)
{
    // 瞌睡怪属性：血量中等，速度慢，防御低，攻击力低
    bool ok = initEnemy("SleepyMonster",
        imagePath,
        startPosition,
        80,        // maxHp
        50.0f,     // speed (慢)
        5,         // defense
        15,        // attackDamage
        40.0f,     // attackRange
        30);       // expReward
    if (!ok) return false;

    setTargetPlayer(target);
    _pauseTimer = 0.0f;
    _isPausing = false;
    changeRandomDirection();
    return true;
}

void SleepyMonster::changeRandomDirection()
{
    float angle = CCRANDOM_0_1() * 2 * M_PI;
    _randomDirection = Vec2(cos(angle), sin(angle));
    _randomDirection.normalize();
}

void SleepyMonster::move(float dt)
{
    if (_targetPlayer == nullptr) return;

    // 瞌睡怪行为：每隔一段时间停顿或随机移动
    if (_isPausing)
    {
        _pauseTimer -= dt;
        if (_pauseTimer <= 0.0f)
        {
            _isPausing = false;
            changeRandomDirection();
        }
        else
        {
            // 停顿期间不移动
            return;
        }
    }
    else
    {
        // 随机移动一段时间
        _pauseTimer -= dt;
        if (_pauseTimer <= 0.0f)
        {
            // 随机停顿 0.5~1.5 秒
            _isPausing = true;
            _pauseTimer = 0.5f + CCRANDOM_0_1() * 1.0f;
            return;
        }

        // 按照随机方向移动
        Vec2 newPos = getPosition() + _randomDirection * getSpeed() * dt;
        setPosition(newPos);
    }

    // 偶尔（1%概率每帧）改变随机方向，增加不可预测性
    if (CCRANDOM_0_1() < 0.01f)
    {
        changeRandomDirection();
    }
}

void SleepyMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    // 瞌睡怪攻击力较低
    _targetPlayer->takeDamage(_attackDamage);
    // 可以播放音效或粒子效果（后续由其他组员实现）
}