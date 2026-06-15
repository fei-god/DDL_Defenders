#pragma once
#ifndef __THESIS_BOSS_H__
#define __THESIS_BOSS_H__

#include "BossMonster.h"

class ThesisBoss : public BossMonster
{
public:
    static ThesisBoss* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel = 1);

    bool initThesisBoss(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel);

    virtual void move(float dt) override;
    virtual void attack() override;
};

#endif
