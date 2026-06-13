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

    // 每帧更新
    virtual void updateObject(float dt) override;
    virtual void updatePlayer(float dt);

    // =========================
    // 移动输入
    // =========================

    virtual void movePlayer(float dt);

    virtual void setInputDirection(const cocos2d::Vec2& direction);
    virtual cocos2d::Vec2 getInputDirection() const;

    virtual bool getIsMoving() const;

    // =========================
    // 玩家受伤 / 死亡
    // =========================

    virtual void takeDamage(int damage) override;
    virtual void takeDamage(int damage, DamageType damageType, Role* attacker = nullptr) override;

    virtual void die() override;

    // =========================
    // 情绪系统
    // =========================

    virtual void changeMood(MoodType mood);
    virtual void changeMood(MoodType mood, float duration);

    virtual void applyMoodEffect();

    virtual MoodType getCurrentMood() const;
    virtual std::string getCurrentMoodName() const;

    virtual MoodSystem* getMoodSystem();

    // =========================
    // 速度
    // =========================

    virtual float getBaseSpeed() const;
    virtual void setBaseSpeed(float speed);

    virtual float getCurrentSpeed() const override;

    // =========================
    // 无敌状态
    // =========================

    virtual void setInvincible(bool invincible);
    virtual void setInvincibleTime(float time);

    virtual bool getInvincible() const;
    virtual float getInvincibleTime() const;

    // =========================
    // 经验和等级
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
    // 材料 / 金币
    // Brotato 里更接近 materials，这里用 material 表示局内货币
    // =========================

    virtual void addMaterial(int amount);
    virtual bool spendMaterial(int amount);

    virtual int getMaterial() const;
    virtual void setMaterial(int amount);

    // 兼容你后面如果想叫金币
    virtual void addGold(int amount);
    virtual bool spendGold(int amount);
    virtual int getGold() const;

    // =========================
    // 玩家经济 / 成长属性
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

    virtual float getEnemySpeedModifier() const;
    virtual void setEnemySpeedModifier(float value);

    // =========================
    // 武器槽
    // 用 GameObject* 是为了不强绑定 Weapon 类，避免循环 include
    // 后面 Weapon 继承 GameObject 时可以直接放进来
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
    // 自动瞄准 / 目标
    // =========================

    virtual void setTarget(GameObject* target);
    virtual GameObject* getTarget() const;

    virtual bool hasTarget() const;
    virtual void clearTarget();

    virtual GameObject* findNearestTarget(const std::vector<GameObject*>& candidates, float searchRange) const;

    // =========================
    // 拾取与交互
    // =========================

    virtual bool canPickup(GameObject* item) const;
    virtual bool isItemInPickupRange(GameObject* item) const;

    virtual void pickupExp(int amount);
    virtual void pickupMaterial(int amount);
    virtual void pickupHeal(int amount);

    // =========================
    // 波次开始 / 结束
    // =========================

    virtual void onWaveStart();
    virtual void onWaveEnd();

    // =========================
    // 玩家属性总览
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

    // 例如某些道具会让敌人速度 +10% 或 -10%
    float enemySpeedModifier;

    int maxWeaponCount;
    std::vector<GameObject*> weapons;

    GameObject* currentTarget;

    float hpRegenTimer;
};

#endif
