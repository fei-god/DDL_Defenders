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
    // 瞌睡怪物：低血量，慢速度，低攻击，随机移动
    bool ok = initEnemy("SleepyMonster",
        imagePath,
        startPosition,
        35,        // maxHp       (降低：80→35，之前太肉了)
        55.0f,     // speed       (降低：50→55)
        3,         // defense     (降低：5→3)
        10,        // attackDamage (降低：15→10)
        35.0f,     // attackRange  (降低：40→35)
        20);       // expReward    (降低：30→20)
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

    // 瞌睡行为：每隔一段时间停顿或随机移动
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

        // 沿随机方向缓慢移动
        // 同时略微向玩家靠近，不会完全跑偏
        Vec2 dirToPlayer = _targetPlayer->getPosition() - getPosition();
        dirToPlayer.normalize();
        Vec2 blendedDir = (_randomDirection * 0.6f + dirToPlayer * 0.4f);
        blendedDir.normalize();
        setDirection(blendedDir);

        Vec2 newPos = getPosition() + blendedDir * getSpeed() * dt;
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
    // 瞌睡怪物攻击力较低
    _targetPlayer->takeDamage(_attackDamage);
    // 可以添加减速效果，由其他组员实现
}
