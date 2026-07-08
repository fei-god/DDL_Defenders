#pragma once
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Role.h"
#include "MoodSystem.h"
#include <vector>
#include <string>

class Player : public Role
{
public:
    Player();
    virtual ~Player();

    // ��ʼ�����
    virtual bool initPlayer(
        const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float baseSpeed,
        int defense
    );

    // �������
    static Player* create(
        const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float baseSpeed,
        int defense
    );

    // ÿ֡����
    virtual void updateObject(float dt) override;
    virtual void updatePlayer(float dt);

    // =========================
    // �ƶ�����
    // =========================

    virtual void movePlayer(float dt);

    virtual void setInputDirection(const cocos2d::Vec2& direction);
    virtual cocos2d::Vec2 getInputDirection() const;

    virtual bool getIsMoving() const;

    // =========================
    // ������� / ����
    // =========================

    virtual void takeDamage(int damage) override;
    virtual void takeDamage(int damage, DamageType damageType, Role* attacker = nullptr) override;

    virtual void die() override;

    // =========================
    // ����ϵͳ
    // =========================

    virtual void changeMood(MoodType mood);
    virtual void changeMood(MoodType mood, float duration);

    virtual void applyMoodEffect();

    virtual MoodType getCurrentMood() const;
    virtual std::string getCurrentMoodName() const;

    virtual MoodSystem* getMoodSystem();

    // =========================
    // �ٶ�
    // =========================

    virtual float getBaseSpeed() const;
    virtual void setBaseSpeed(float speed);

    virtual float getCurrentSpeed() const override;

    // =========================
    // �޵�״̬
    // =========================

    virtual void setInvincible(bool invincible);
    virtual void setInvincibleTime(float time);

    virtual bool getInvincible() const;
    virtual float getInvincibleTime() const;

    // =========================
    // ����͵ȼ�
    // =========================

    virtual void addExp(int amount);
    virtual int getExp() const;

    virtual int getLevel() const;
    virtual void setLevel(int level);

    virtual int getExpToNextLevel() const;
    virtual float getExpPercent() const;

    virtual bool canLevelUp() const;
    virtual void levelUp();

    virtual int getUpgradePoints() const;
    virtual void addUpgradePoint(int amount);
    virtual bool spendUpgradePoint(int amount = 1);

    // =========================
    // ���� / ���
    // Brotato ����ӽ� materials�������� material ��ʾ���ڻ���
    // =========================

    virtual void addMaterial(int amount);
    virtual bool spendMaterial(int amount);

    virtual int getMaterial() const;
    virtual void setMaterial(int amount);

    // ��������������н��
    virtual void addGold(int amount);
    virtual bool spendGold(int amount);
    virtual int getGold() const;

    // =========================
    // ��Ҿ��� / �ɳ�����
    // =========================

    virtual int getHarvesting() const;
    virtual void setHarvesting(int value);
    virtual void addHarvesting(int value);

    virtual float getLuck() const;
    virtual void setLuck(float value);
    virtual void addLuck(float value);

    virtual float getPickupRange() const;
    virtual void setPickupRange(float range);
    virtual void addPickupRange(float value);

    virtual float getHpRegen() const;
    virtual void setHpRegen(float value);
    virtual void addHpRegen(float value);



    // =========================
    // ������
    // �� GameObject* ��Ϊ�˲�ǿ�� Weapon �࣬����ѭ�� include
    // ���� Weapon �̳� GameObject ʱ����ֱ�ӷŽ���
    // =========================

    virtual int getMaxWeaponCount() const;
    virtual void setMaxWeaponCount(int count);

    virtual bool canAddWeapon() const;
    virtual bool addWeapon(GameObject* weapon);
    virtual bool removeWeapon(GameObject* weapon);
    virtual void clearWeapons();

    virtual const std::vector<GameObject*>& getWeapons() const;
    virtual int getWeaponCount() const;

    // =========================
    // �Զ���׼ / Ŀ��
    // =========================

    virtual void setTarget(GameObject* target);
    virtual GameObject* getTarget() const;

    virtual bool hasTarget() const;
    virtual void clearTarget();

    virtual GameObject* findNearestTarget(const std::vector<GameObject*>& candidates, float searchRange) const;

    // =========================
    // ʰȡ�뽻��
    // =========================

    virtual bool canPickup(GameObject* item) const;
    virtual bool isItemInPickupRange(GameObject* item) const;


    // =========================
    // ���ο�ʼ / ����
    // =========================



    // =========================
    // �����������
    // =========================

    virtual std::string getPlayerStatsInfo() const;
    virtual std::string getDebugInfo() const override;

protected:
    MoodSystem moodSystem;

    float baseSpeed;
    float currentSpeed;

    cocos2d::Vec2 inputDirection;
    bool isMoving;

    bool isInvincible;
    float invincibleTime;
    float maxInvincibleTime;

    int exp;
    int level;
    int expToNextLevel;
    int upgradePoints;

    int material;
    int gold;

    int harvesting;
    float luck;
    float pickupRange;
    float hpRegen;

    int maxWeaponCount;
    std::vector<GameObject*> weapons;

    GameObject* currentTarget;

    float hpRegenTimer;
};

#endif
