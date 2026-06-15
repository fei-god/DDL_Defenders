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
    float _chargeCooldown;     // �����ȴ
    bool _isCharging;          // �Ƿ����ڳ��
    cocos2d::Vec2 _chargeDirection; // ��̷���
};
#endif