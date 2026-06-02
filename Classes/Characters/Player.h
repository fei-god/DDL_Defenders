#pragma once
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Role.h"
#include "MoodSystem.h"

class Player : public Role
{
public:
    Player();
    virtual ~Player();

    // 初始化玩家
    virtual bool initPlayer(
        const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float baseSpeed,
        int defense
    );

    // 创建玩家
    static Player* create(
        const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float baseSpeed,
        int defense
    );

    // 每帧更新玩家
    virtual void updatePlayer(float dt);

    // 处理移动
    virtual void movePlayer(float dt);

    // 键盘输入方向
    virtual void setInputDirection(const cocos2d::Vec2& direction);
    virtual cocos2d::Vec2 getInputDirection() const;

    // 玩家受伤
    virtual void takeDamage(int damage) override;

    // 玩家死亡
    virtual void die() override;

    // 改变情绪
    virtual void changeMood(MoodType mood);

    // 改变情绪，并设置持续时间
    virtual void changeMood(MoodType mood, float duration);

    // 应用情绪效果
    virtual void applyMoodEffect();

    // 获取当前情绪
    virtual MoodType getCurrentMood() const;

    // 获取当前情绪名字
    virtual std::string getCurrentMoodName() const;

    // 获取当前实际速度
    virtual float getCurrentSpeed() const;

    // 获取情绪系统
    virtual MoodSystem* getMoodSystem();

    // 无敌状态
    virtual void setInvincible(bool invincible);
    virtual bool getInvincible() const;

    // 经验和等级
    virtual void addExp(int amount);
    virtual int getExp() const;
    virtual int getLevel() const;

protected:
    MoodSystem moodSystem;             // 情绪系统

    float baseSpeed;                   // 基础速度
    float currentSpeed;                // 当前实际速度

    cocos2d::Vec2 inputDirection;      // 输入方向
    bool isMoving;                     // 是否正在移动

    bool isInvincible;                 // 是否无敌
    float invincibleTime;              // 当前无敌剩余时间
    float maxInvincibleTime;           // 最大无敌时间

    int exp;                           // 经验
    int level;                         // 等级
};

#endif
