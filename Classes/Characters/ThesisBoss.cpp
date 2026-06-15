#include "ThesisBoss.h"

USING_NS_CC;

ThesisBoss* ThesisBoss::create(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    ThesisBoss* boss = new (std::nothrow) ThesisBoss();
    if (boss && boss->initThesisBoss(imagePath, startPosition, target, waveLevel))
    {
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}

bool ThesisBoss::initThesisBoss(const std::string& imagePath,
    const Vec2& startPosition,
    Player* target,
    int waveLevel)
{
    if (!BossMonster::initBossMonster(imagePath, startPosition, target, waveLevel))
    {
        return false;
    }

    setObjectName("ThesisBoss");
    addTag("Boss");
    addTag("Thesis");
    return true;
}

void ThesisBoss::move(float dt)
{
    BossMonster::move(dt);
}

void ThesisBoss::attack()
{
    BossMonster::attack();
    if (getTargetPlayer())
    {
        getTargetPlayer()->changeMood(MoodType::Exhausted, 1.5f);
    }
}
