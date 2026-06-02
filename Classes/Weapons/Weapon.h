#pragma once
#ifndef WEAPON_H
#define WEAPON_H

#include "GameObject.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
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

    // 每帧更新武器冷却，冷却结束后自动开火。
    virtual void updateObject(float dt) override;

    virtual void fire() = 0;

    int getAttackPower() const;
    float getCooldownTime() const;

protected:
    bool canFire() const;
    void resetCooldown();

    Enemy* findNearestEnemy() const;
    cocos2d::Vec2 getDirectionToEnemy(Enemy* enemy) const;

protected:
    Player* _owner;                       // 武器拥有者，通常是玩家
    vector<Enemy*>* _enemyList;           // 敌人列表
    vector<Bullet*>* _bulletList;           // 子弹列表
    cocos2d::Node* _bulletLayer;           // 子弹所在层

    int _attackPower;                      // 武器攻击力
    float _cooldownTime;                   // 基础冷却时间
    float _cooldownTimer;                  // 当前剩余冷却

    float _bulletSpeed;                    // 子弹速度
	string _bulletImagePath;               //子弹图片路径
};

#endif