#include "SleepyMonster.h"
#include "cocos2d.h"

USING_NS_CC;

SleepyMonster* SleepyMonster::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    SleepyMonster* monster = new SleepyMonster();
    if (monster && monster->initSleepyMonster(imagePath, startPosition, target, waveLevel))
    {
        monster->autorelease();
        return monster;
    }
    CC_SAFE_DELETE(monster);
    return nullptr;
}

bool SleepyMonster::initSleepyMonster(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    // Sleepy monster: low HP, slow speed, low attack, random movement
    // Stats scale with wave level
    int scaledHp = 20 + waveLevel * 4;
    float scaledSpeed = 82.0f + waveLevel * 2.5f;
    int scaledAtk = 7 + waveLevel * 1;
    int scaledExp = 20 + waveLevel * 5;
    bool ok = initEnemy("SleepyMonster",
        imagePath,
        startPosition,
        scaledHp,
        scaledSpeed,
        0,
        scaledAtk,
        35.0f,
        scaledExp);
    if (!ok) return false;

    setTargetPlayer(target);
    _pauseTimer = 0.7f;
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
            Vec2 dirToPlayer = _targetPlayer->getPosition() - getPosition();
            if (dirToPlayer.lengthSquared() > 0.0001f)
            {
                dirToPlayer.normalize();
                setDirection(dirToPlayer);
                setPosition(getPosition() + dirToPlayer * getSpeed() * 0.35f * dt);
            }
            return;
        }
    }
    else
    {
        _pauseTimer -= dt;
        if (_pauseTimer <= 0.0f)
        {
            _isPausing = true;
            _pauseTimer = 0.12f + CCRANDOM_0_1() * 0.18f;
            return;
        }

        Vec2 dirToPlayer = _targetPlayer->getPosition() - getPosition();
        dirToPlayer.normalize();
        Vec2 blendedDir = (_randomDirection * 0.18f + dirToPlayer * 0.82f);
        blendedDir.normalize();
        setDirection(blendedDir);

        Vec2 newPos = getPosition() + blendedDir * getSpeed() * dt;
        setPosition(newPos);
    }

    if (CCRANDOM_0_1() < 0.01f)
    {
        changeRandomDirection();
    }
}

void SleepyMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    _targetPlayer->takeDamage(_attackDamage);
}
