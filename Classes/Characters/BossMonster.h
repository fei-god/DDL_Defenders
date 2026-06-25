#pragma once
#ifndef __BOSS_MONSTER_H__
#define __BOSS_MONSTER_H__

#include "Enemy.h"

class BossMonster : public Enemy
{
public:
    static BossMonster* create(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel = 1);

    virtual bool initBossMonster(const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        Player* target,
        int waveLevel);

    virtual void move(float dt) override;
    virtual void attack() override;
    virtual void playAttackEffect() override;

    virtual void die() override;

    bool isEnraged() const;

private:
    float _chargeTimer;
    float _chargeInterval;
    bool _isCharging;
    cocos2d::Vec2 _chargeDir;
    float _chargeDuration;

    float _specialAttackTimer;
    float _specialAttackInterval;

    bool _isEnraged;
    int _waveLevel;

    float _wanderTimer;
    cocos2d::Vec2 _wanderTarget;
    bool _isWandering;

    void performSpecialAttack();
    void startCharge();
    void showAOEIndicator();
    void showEnrageAura();
};

#endif
