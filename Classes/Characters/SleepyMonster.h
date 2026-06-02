#pragma once
#pragma once
#ifndef __SLEEPY_MONSTER_H__
#define __SLEEPY_MONSTER_H__

#include "Enemy.h"

class SleepyMonster : public Enemy
{
public:
    static SleepyMonster* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target);

    virtual bool initSleepyMonster(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target);

    // 实现纯虚函数
    virtual void move(float dt) override;
    virtual void attack() override;

private:
    float _pauseTimer;      // 停顿计时器
    bool _isPausing;        // 是否正在停顿
    cocos2d::Vec2 _randomDirection; // 随机移动方向
    void changeRandomDirection();
};
#endif