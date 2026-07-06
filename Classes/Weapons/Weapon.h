#pragma once
#ifndef WEAPON_H
#define WEAPON_H

#include "GameObject.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "BulletPool.h"
#include <vector>
#include <string>
using namespace std;

class Weapon : public GameObject
{
public:
	Weapon();
	virtual ~Weapon();

    virtual bool initWeapon(
        const string& name,
        const string& imagePath,
        Player* owner,
        int attackPower,
        float cooldownTime
    );

    //绑定战斗数据
    void bindBattleData(
        vector<Enemy*>* enemyList,
        vector<Bullet*>* bulletList,
        cocos2d::Node* bulletLayer
    );
    void bindBulletPool(BulletPool* bulletPool);

    // 每帧更新武器冷却，冷却结束后自动开火。
    virtual void updateObject(float dt) override;
    void updateCooldown(float dt);
    void readyNow();

    virtual void fire() = 0;

    void configureEnergy(float maxEnergy, float energyCost, float recoverPerSecond);
    bool isReadyToFire() const;
    bool hasEnoughEnergy() const;
    float getEnergyRatio() const;
    float getCurrentEnergy() const;
    float getMaxEnergy() const;
    float getEnergyCost() const;
    float getEnergyRecoverPerSecond() const;

    void setAimDirection(const cocos2d::Vec2& direction);
    cocos2d::Vec2 getAimDirection() const;
    std::string getWeaponName() const;
    int getModifiedAttackPower() const;
    Enemy* findNearestEnemy() const;
    cocos2d::Vec2 getMuzzlePosition(const cocos2d::Vec2& direction) const;

    int getAttackPower() const;
    float getCooldownTime() const;
    int getProjectileCountBonus() const;
    float getAttackRange() const;
    void setAttackRange(float range);

    void addAttackPower(int amount);
    void addEnergyRecoverPercent(float percent);
    void addMaxEnergy(float amount);
    void addProjectileCountBonus(int amount);

protected:
    bool canFire() const;
    bool consumeEnergyForShot();
    void resetCooldown();

    cocos2d::Vec2 getDirectionToEnemy(Enemy* enemy) const;
    Bullet* spawnBullet(const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        const cocos2d::Vec2& direction,
        float speed,
        int damage,
        float lifeTime,
        bool canPierce);

protected:
    Player* _owner;                       // 武器拥有者，通常是玩家
    vector<Enemy*>* _enemyList;           // 敌人列表
    vector<Bullet*>* _bulletList;           // 子弹列表
    cocos2d::Node* _bulletLayer;           // 子弹所在层
    BulletPool* _bulletPool;

    int _attackPower;                      // 武器攻击力
    float _attackRange;                    // 自动攻击索敌范围
    float _cooldownTime;                   // 基础冷却时间
    float _cooldownTimer;                  // 当前剩余冷却
    float _maxEnergy;
    float _currentEnergy;
    float _energyCost;
    float _energyRecoverPerSecond;
    int _projectileCountBonus;

    float _bulletSpeed;                    // 子弹速度
	string _bulletImagePath;               //子弹图片路径
    cocos2d::Vec2 _aimDirection;
};

#endif
