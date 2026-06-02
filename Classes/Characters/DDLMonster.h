#pragma once
#pragma once
#ifndef __DDL_MONSTER_H__
#define __DDL_MONSTER_H__

#include "Enemy.h"

class DDLMonster : public Enemy
{
public:
    static DDLMonster* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target);

    virtual bool initDDLMonster(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target);

    virtual void move(float dt) override;
    virtual void attack() override;

private:
    float _chargeCooldown;     // 冲刺冷却
    bool _isCharging;          // 是否正在冲刺
    cocos2d::Vec2 _chargeDirection; // 冲刺方向
};
#endif