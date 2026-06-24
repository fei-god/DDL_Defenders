#pragma once
#ifndef __ROLE_H__
#define __ROLE_H__

#include "GameObject.h"
#include <string>
#include <vector>

// �˺�����
// Brotato ����Ϸ��������������ߡ����˹������в�ͬ����
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

// ״̬Ч������
enum class RoleStatusType
{
    None,
    Burning,
    Poison,
    Slow,
    Stun,
    Invincible
};

// ����״̬Ч��
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

    // ��ʼ�� Role ��������
    virtual bool initRole(
        const std::string& name,
        GameObjectType type,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float speed,
        int defense
    );

    // ÿ֡����
    virtual void updateObject(float dt) override;

    // =========================
    // �ƶ�
    // =========================

    virtual void move(float dt);

    virtual void setDirection(const cocos2d::Vec2& direction);
    virtual cocos2d::Vec2 getDirection() const;

    virtual void stopMove();

    // =========================
    // ���ˡ���Ѫ������
    // =========================

    virtual void takeDamage(int damage);
    virtual void takeDamage(int damage, DamageType damageType, Role* attacker = nullptr);

    virtual int calculateFinalDamage(int damage, DamageType damageType) const;

    virtual void heal(int amount);
    virtual void healByPercent(float percent);

    virtual bool isDead() const;
    virtual void die();

    virtual bool canTakeDamage() const;

    // �޵�ʱ�䣬������ұ�����ײ��������޵�
    virtual void setInvincible(float duration);
    virtual bool isInvincibleNow() const;
    virtual void updateHurtCooldown(float dt);

    // =========================
    // HP
    // =========================

    virtual int getHp() const;
    virtual void setHp(int hp);

    virtual int getMaxHp() const;
    virtual void setMaxHp(int maxHp);

    virtual float getHpPercent() const;

    // =========================
    // ��������
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
    // Brotato ���ս������
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

    // �����˺����ͻ�ù����ӳ�
    virtual int getDamageBonusByType(DamageType damageType) const;

    // �����ж�����������˺�
    virtual bool rollCritical() const;
    virtual int calculateOutgoingDamage(int baseDamage, DamageType damageType) const;

    // �����ж�
    virtual bool rollDodge() const;

    // ����͵ȡ����
    virtual void tryLifeSteal(int damageDealt);

    // =========================
    // ״̬Ч��
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
    // �ܻ�����
    // =========================

    virtual void applyKnockback(const cocos2d::Vec2& fromPosition, float force);
    virtual void flashWhenHit();

    // =========================
    // �����޸Ľӿ�
    // ��װ����������MoodSystem ʹ��
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
    // ����
    // =========================

    virtual std::string getDebugInfo() const override;

protected:
    // =========================
    // ��������
    // =========================

    int hp;
    int maxHp;

    float speed;
    cocos2d::Vec2 direction;

    bool isAlive;

    // ��ԭ���� defense ����
    // armor ��Ϊ������ Brotato �Ļ��׸���
    int defense;
    int armor;

    // =========================
    // ս������
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
    // ���˿���
    // =========================

    float invincibleTimer;
    float hurtCooldown;
    float hurtCooldownTimer;

    // =========================
    // ״̬Ч��
    // =========================

    std::vector<RoleStatusEffect> statusEffects;
};

#endif
