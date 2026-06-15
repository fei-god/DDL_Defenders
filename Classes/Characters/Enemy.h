#pragma once
#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Role.h"
#include "Player.h"   // ǰ������Ҳ���ԣ�����Ҫ֪�� Player ��

class Enemy : public Role
{
public:
    Enemy();
    virtual ~Enemy();

    // ��ʼ������
    virtual bool initEnemy(const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float speed,
        int defense,
        int attackDamage,
        float attackRange,
        int expReward);

    // ���麯����ǿ��������ʵ���Լ����ƶ��͹�����Ϊ�����ֶ�̬��
    virtual void move(float dt) = 0;
    virtual void attack() = 0;

    // ����/��ȡĿ����ң����� AI ׷��
    void setTargetPlayer(Player* player);
    Player* getTargetPlayer() const;

    // ��������������Χ�����齱���Ĵ�ȡ
    void setAttackDamage(int damage);
    int getAttackDamage() const;
    void setAttackRange(float range);
    float getAttackRange() const;
    void setExpReward(int exp);
    int getExpReward() const;

    // ��������ʱ����������Ӿ���
    virtual void die() override;
    virtual void takeDamage(int damage) override;
    virtual void takeDamage(int damage, DamageType damageType, Role* attacker = nullptr) override;

    // ���ⲿ���ã�ÿ֡���µ��˵��ƶ��͹�����ȴ
    void updateEnemy(float dt);

protected:
    Player* _targetPlayer;      // ���ָ�루�����ã�����Ҫ�ͷţ�
    int _attackDamage;          // ������
    float _attackRange;         // ������Χ
    int _expReward;             // ���ܺ����ľ���ֵ
    int _hitCount;
    int _hitsToDie;

    virtual int getHitsToDieForPlayer(Player* player) const;

    float _attackCooldown;      // ������ȴʣ��ʱ��
    float _attackCooldownMax;   // ������ȴ���ֵ��Ĭ��Ϊ1�룩
};

#endif