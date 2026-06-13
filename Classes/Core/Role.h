#pragma once
#ifndef __ROLE_H__
#define __ROLE_H__

#include "GameObject.h"
#include <string>
#include <vector>

// 伤害类型
// Brotato 类游戏里后期武器、道具、敌人攻击会有不同类型
enum class DamageType
{
    Normal,
    Melee,
    Ranged,
    Elemental,
    Burning,
    Poison,
    TrueDamage
};

// 状态效果类型
enum class RoleStatusType
{
    None,
    Burning,
    Poison,
    Slow,
    Stun,
    Invincible
};

// 单个状态效果
struct RoleStatusEffect
{
    RoleStatusType type;
    float duration;
    float timer;
    float value;
    float tickInterval;
    float tickTimer;

    RoleStatusEffect()
        : type(RoleStatusType::None)
        , duration(0.0f)
        , timer(0.0f)
        , value(0.0f)
        , tickInterval(1.0f)
        , tickTimer(0.0f)
    {
    }

    RoleStatusEffect(RoleStatusType type, float duration, float value, float tickInterval = 1.0f)
        : type(type)
        , duration(duration)
        , timer(0.0f)
        , value(value)
        , tickInterval(tickInterval)
        , tickTimer(0.0f)
    {
    }
};

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

    // 每帧更新
    virtual void updateObject(float dt) override;

    // =========================
    // 移动
    // =========================

    virtual void move(float dt);

    virtual void setDirection(const cocos2d::Vec2& direction);
    virtual cocos2d::Vec2 getDirection() const;

    virtual void stopMove();

    // =========================
    // 受伤、回血、死亡
    // =========================

    virtual void takeDamage(int damage);
    virtual void takeDamage(int damage, DamageType damageType, Role* attacker = nullptr);

    virtual int calculateFinalDamage(int damage, DamageType damageType) const;

    virtual void heal(int amount);
    virtual void healByPercent(float percent);

    virtual bool isDead() const;
    virtual void die();

    virtual bool canTakeDamage() const;

    // 无敌时间，例如玩家被敌人撞到后短暂无敌
    virtual void setInvincible(float duration);
    virtual bool isInvincibleNow() const;

    // =========================
    // HP
    // =========================

    virtual int getHp() const;
    virtual void setHp(int hp);

    virtual int getMaxHp() const;
    virtual void setMaxHp(int maxHp);

    virtual float getHpPercent() const;

    // =========================
    // 基础属性
    // =========================

    virtual float getSpeed() const;
    virtual void setSpeed(float speed);

    virtual float getCurrentSpeed() const;

    virtual int getDefense() const;
    virtual void setDefense(int defense);

    virtual int getArmor() const;
    virtual void setArmor(int armor);

    virtual bool isRoleAlive() const;

    // =========================
    // Brotato 风格战斗属性
    // =========================

    virtual int getBaseDamage() const;
    virtual void setBaseDamage(int damage);

    virtual int getMeleeDamage() const;
    virtual void setMeleeDamage(int damage);

    virtual int getRangedDamage() const;
    virtual void setRangedDamage(int damage);

    virtual int getElementalDamage() const;
    virtual void setElementalDamage(int damage);

    virtual float getAttackSpeed() const;
    virtual void setAttackSpeed(float attackSpeed);

    virtual float getCritChance() const;
    virtual void setCritChance(float critChance);

    virtual float getCritDamageMultiplier() const;
    virtual void setCritDamageMultiplier(float multiplier);

    virtual float getDodgeChance() const;
    virtual void setDodgeChance(float dodgeChance);

    virtual float getLifeStealChance() const;
    virtual void setLifeStealChance(float chance);

    virtual float getKnockbackResistance() const;
    virtual void setKnockbackResistance(float resistance);

    // 根据伤害类型获得攻击加成
    virtual int getDamageBonusByType(DamageType damageType) const;

    // 暴击判断与最终输出伤害
    virtual bool rollCritical() const;
    virtual int calculateOutgoingDamage(int baseDamage, DamageType damageType) const;

    // 闪避判断
    virtual bool rollDodge() const;

    // 生命偷取触发
    virtual void tryLifeSteal(int damageDealt);

    // =========================
    // 状态效果
    // =========================

    virtual void addStatusEffect(RoleStatusType type, float duration, float value, float tickInterval = 1.0f);
    virtual void removeStatusEffect(RoleStatusType type);
    virtual bool hasStatusEffect(RoleStatusType type) const;
    virtual void clearStatusEffects();

    virtual void updateStatusEffects(float dt);

    virtual void applyBurning(float duration, float damagePerTick);
    virtual void applyPoison(float duration, float damagePerTick);
    virtual void applySlow(float duration, float slowPercent);
    virtual void applyStun(float duration);

    virtual bool isStunned() const;
    virtual bool isSlowed() const;

    // =========================
    // 受击反馈
    // =========================

    virtual void applyKnockback(const cocos2d::Vec2& fromPosition, float force);
    virtual void flashWhenHit();

    // =========================
    // 属性修改接口
    // 给装备、升级、MoodSystem 使用
    // =========================

    virtual void addMaxHp(int value);
    virtual void addDefense(int value);
    virtual void addArmor(int value);
    virtual void addSpeed(float value);
    virtual void addBaseDamage(int value);
    virtual void addMeleeDamage(int value);
    virtual void addRangedDamage(int value);
    virtual void addElementalDamage(int value);
    virtual void addAttackSpeed(float value);
    virtual void addCritChance(float value);
    virtual void addDodgeChance(float value);

    // =========================
    // 调试
    // =========================

    virtual std::string getDebugInfo() const override;

protected:
    // =========================
    // 生存属性
    // =========================

    int hp;
    int maxHp;

    float speed;
    cocos2d::Vec2 direction;

    bool isAlive;

    // 你原来的 defense 保留
    // armor 是为了贴近 Brotato 的护甲概念
    int defense;
    int armor;

    // =========================
    // 战斗属性
    // =========================

    int baseDamage;
    int meleeDamage;
    int rangedDamage;
    int elementalDamage;

    float attackSpeed;
    float critChance;
    float critDamageMultiplier;
    float dodgeChance;
    float lifeStealChance;
    float knockbackResistance;

    // =========================
    // 受伤控制
    // =========================

    float invincibleTimer;
    float hurtCooldown;
    float hurtCooldownTimer;

    // =========================
    // 状态效果
    // =========================

    std::vector<RoleStatusEffect> statusEffects;
};

#endif
