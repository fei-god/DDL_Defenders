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

    // ʵ�ִ��麯��
    virtual void move(float dt) override;
    virtual void attack() override;

private:
    float _pauseTimer;      // ͣ�ټ�ʱ��
    bool _isPausing;        // �Ƿ�����ͣ��
    cocos2d::Vec2 _randomDirection; // ����ƶ�����
    void changeRandomDirection();
};
#endif