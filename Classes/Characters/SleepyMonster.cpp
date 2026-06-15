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
    // 鐬岀潯鎬墿锛氫綆琛€閲忥紝鎱㈤€熷害锛屼綆鏀诲嚮锛岄殢鏈虹Щ鍔?
    bool ok = initEnemy("SleepyMonster",
        imagePath,
        startPosition,
        20,        // maxHp
        82.0f,     // speed
        0,         // defense
        7,         // attackDamage
        35.0f,     // attackRange  (闄嶄綆锛?0鈫?5)
        20);       // expReward    (闄嶄綆锛?0鈫?0)
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

    // 鐬岀潯琛屼负锛氭瘡闅斾竴娈垫椂闂村仠椤挎垨闅忔満绉诲姩
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
        // 闅忔満绉诲姩涓€娈垫椂闂?
        _pauseTimer -= dt;
        if (_pauseTimer <= 0.0f)
        {
            // 闅忔満鍋滈】 0.5~1.5 绉?
            _isPausing = true;
            _pauseTimer = 0.12f + CCRANDOM_0_1() * 0.18f;
            return;
        }

        // 娌块殢鏈烘柟鍚戠紦鎱㈢Щ鍔?
        // 鍚屾椂鐣ュ井鍚戠帺瀹堕潬杩戯紝涓嶄細瀹屽叏璺戝亸
        Vec2 dirToPlayer = _targetPlayer->getPosition() - getPosition();
        dirToPlayer.normalize();
        Vec2 blendedDir = (_randomDirection * 0.18f + dirToPlayer * 0.82f);
        blendedDir.normalize();
        setDirection(blendedDir);

        Vec2 newPos = getPosition() + blendedDir * getSpeed() * dt;
        setPosition(newPos);
    }

    // 鍋跺皵锛?%姒傜巼姣忓抚锛夋敼鍙橀殢鏈烘柟鍚戯紝澧炲姞涓嶅彲棰勬祴鎬?
    if (CCRANDOM_0_1() < 0.01f)
    {
        changeRandomDirection();
    }
}

void SleepyMonster::attack()
{
    if (_targetPlayer == nullptr) return;
    // 鐬岀潯鎬墿鏀诲嚮鍔涜緝浣?
    _targetPlayer->takeDamage(_attackDamage);
    // 鍙互娣诲姞鍑忛€熸晥鏋滐紝鐢卞叾浠栫粍鍛樺疄鐜?
}


