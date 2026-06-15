#pragma once
#ifndef __PHONE_MONSTER_H__
#define __PHONE_MONSTER_H__

#include "Enemy.h"

class PhoneMonster : public Enemy
{
public:
    static PhoneMonster* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target);

    bool initPhoneMonster(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target);

    virtual void move(float dt) override;
    virtual void attack() override;

private:
    float _zigzagTimer;
    float _zigzagSign;
};

#endif
