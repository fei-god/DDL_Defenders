#pragma once
#ifndef __SLEEPY_MONSTER_H__
#define __SLEEPY_MONSTER_H__

#include "Enemy.h"

class SleepyMonster : public Enemy
{
public:
    static SleepyMonster* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel = 1);

    virtual bool initSleepyMonster(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel = 1);

    virtual void move(float dt) override;
    virtual void attack() override;
    virtual void playAttackEffect() override;

private:
    float _pauseTimer;
    bool _isPausing;
    cocos2d::Vec2 _randomDirection;
    void changeRandomDirection();
};
#endif
