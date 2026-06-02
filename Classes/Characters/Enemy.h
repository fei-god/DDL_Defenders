#pragma once
#pragma once
#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Role.h"
#include "Player.h"   // 前向声明也可以，但需要知道 Player 类

class Enemy : public Role
{
public:
    Enemy();
    virtual ~Enemy();

    // 初始化敌人
    virtual bool initEnemy(const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float speed,
        int defense,
        int attackDamage,
        float attackRange,
        int expReward);

    // 纯虚函数：强制派生类实现自己的移动和攻击行为（体现多态）
    virtual void move(float dt) = 0;
    virtual void attack() = 0;

    // 设置/获取目标玩家（用于 AI 追逐）
    void setTargetPlayer(Player* player);
    Player* getTargetPlayer() const;

    // 攻击力、攻击范围、经验奖励的存取
    void setAttackDamage(int damage);
    int getAttackDamage() const;
    void setAttackRange(float range);
    float getAttackRange() const;
    void setExpReward(int exp);
    int getExpReward() const;

    // 敌人死亡时，给玩家增加经验
    virtual void die() override;

    // 供外部调用，每帧更新敌人的移动和攻击冷却
    void updateEnemy(float dt);

protected:
    Player* _targetPlayer;      // 玩家指针（弱引用，不需要释放）
    int _attackDamage;          // 攻击力
    float _attackRange;         // 攻击范围
    int _expReward;             // 击败后奖励的经验值

    float _attackCooldown;      // 攻击冷却剩余时间
    float _attackCooldownMax;   // 攻击冷却最大值（默认为1秒）
};

#endif