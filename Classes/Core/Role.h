#pragma once
#ifndef __ROLE_H__
#define __ROLE_H__

#include "GameObject.h"

class Role : public GameObject
{
public:
    Role();
    virtual ~Role();

    // 初始化 Role 基础属性
    virtual bool initRole(
        const std::string& name,
        GameObjectType type,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float speed,
        int defense
    );

    // 基础移动函数
    virtual void move(float dt);

    // 设置 / 获取方向
    virtual void setDirection(const cocos2d::Vec2& direction);
    virtual cocos2d::Vec2 getDirection() const;

    // 受伤
    virtual void takeDamage(int damage);

    // 回血
    virtual void heal(int amount);

    // 判断是否死亡
    virtual bool isDead() const;

    // 死亡处理
    virtual void die();

    // 获取 / 设置 HP
    virtual int getHp() const;
    virtual void setHp(int hp);

    virtual int getMaxHp() const;
    virtual void setMaxHp(int maxHp);

    // 获取 / 设置速度
    virtual float getSpeed() const;
    virtual void setSpeed(float speed);

    // 获取 / 设置防御力
    virtual int getDefense() const;
    virtual void setDefense(int defense);

    // 判断是否存活
    virtual bool isRoleAlive() const;

protected:
    int hp;                         // 当前血量
    int maxHp;                      // 最大血量
    float speed;                    // 移动速度
    cocos2d::Vec2 direction;        // 移动方向
    bool isAlive;                   // 是否存活
    int defense;                    // 防御力
};

#endif

