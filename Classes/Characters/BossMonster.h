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

    // 实现纯虚函数
    virtual void move(float dt) override;
    virtual void attack() override;

    // Boss特有：死亡时掉落大量经验
    virtual void die() override;

    // Boss阶段（用于血量低于50%时狂暴）
    bool isEnraged() const;

private:
    // Boss特有属性
    float _chargeTimer;            // 冲锋计时器
    float _chargeInterval;         // 冲锋间隔
    bool _isCharging;              // 是否正在冲锋
    cocos2d::Vec2 _chargeDir;      // 冲锋方向
    float _chargeDuration;         // 冲锋持续时间

    float _specialAttackTimer;     // 特殊攻击计时器
    float _specialAttackInterval;  // 特殊攻击间隔（AOE）

    bool _isEnraged;               // 是否狂暴（血量低于50%）
    int _waveLevel;                // 出现的波次（影响强度）

    float _wanderTimer;            // 徘徊计时器
    cocos2d::Vec2 _wanderTarget;   // 徘徊目标点
    bool _isWandering;             // 是否在徘徊

    void performSpecialAttack();   // 范围攻击
    void startCharge();            // 开始冲锋
};

#endif
