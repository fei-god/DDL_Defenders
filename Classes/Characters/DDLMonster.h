#pragma once
#ifndef __DDL_MONSTER_H__
#define __DDL_MONSTER_H__

#include "Enemy.h"

class DDLMonster : public Enemy
{
public:
    static DDLMonster* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel = 1);

    virtual bool initDDLMonster(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel = 1);

    virtual void move(float dt) override;
    virtual void attack() override;
    virtual void playAttackEffect() override;

private:
    float _chargeCooldown;
    bool _isCharging;
    cocos2d::Vec2 _chargeDirection;
};
#endif
