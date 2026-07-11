#pragma once
#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Role.h"
#include "Player.h"
#include <vector>

// Enemy projectile: visible bullet that flies toward player
struct EnemyProjectile
{
    cocos2d::Node* node;        // Visual node (DrawNode)
    cocos2d::Vec2 position;
    cocos2d::Vec2 direction;    // Normalized
    float speed;
    int damage;
    float lifetime;
    float elapsed;
    bool active;
    float radius;               // Collision radius

    EnemyProjectile()
        : node(nullptr), position(cocos2d::Vec2::ZERO), direction(cocos2d::Vec2::ZERO)
        , speed(200.0f), damage(10), lifetime(3.0f), elapsed(0.0f)
        , active(false), radius(12.0f)
    {}
};

class Enemy : public Role
{
public:
    Enemy();
    virtual ~Enemy();

    virtual bool initEnemy(const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        int maxHp,
        float speed,
        int defense,
        int attackDamage,
        float attackRange,
        int expReward);

    virtual void move(float dt) = 0;
    virtual void attack() = 0;

    void setTargetPlayer(Player* player);
    Player* getTargetPlayer() const;

    void setAttackDamage(int damage);
    int getAttackDamage() const;
    void setAttackRange(float range);
    float getAttackRange() const;
    void setExpReward(int exp);
    int getExpReward() const;
    void setAttackCooldown(float cd) { _attackCooldown = cd; }

    virtual void die() override;
    virtual void takeDamage(int damage) override;
    virtual void takeDamage(int damage, DamageType damageType, Role* attacker = nullptr) override;
    virtual cocos2d::Rect getCollisionBox() const override;

    void updateEnemy(float dt);

    // Idle animation (Brotato-style bobbing)
    void startIdleAnimation();
    void stopIdleAnimation();

    // Attack telegraph (flash warning so player can dodge)
    void playAttackTelegraph();

    // Attack visual effect (override in derived classes)
    virtual void playAttackEffect();

    // Hit flash effect
    void playHitEffect();

    // Get time scaling factor (monsters get stronger over time)
    static float getTimeScaleFactor(float elapsedTime);

    // --- Enemy Projectile System ---
    // Fire a visible projectile toward the player (or in a direction)
    // Returns the projectile for tracking
    void fireProjectile(const cocos2d::Vec2& direction, float speed,
        const cocos2d::Color4F& glowColor, const cocos2d::Color4F& trailColor,
        int damage, float radius = 10.0f, float lifetime = 3.0f);

    // Fire a projectile aimed at the player
    void fireProjectileAtPlayer(float speed,
        const cocos2d::Color4F& glowColor, const cocos2d::Color4F& trailColor,
        int damage, float radius = 10.0f, float lifetime = 3.0f);

    // Get all active projectiles from this enemy
    std::vector<EnemyProjectile>& getProjectiles() { return _projectiles; }

    // Update projectile positions (called by WaveManager)
    void updateProjectiles(float dt);

    // Clean up dead projectiles
    void cleanupProjectiles();

    // --- Knockback System ---
    void applyKnockback(const cocos2d::Vec2& bulletDir);
    bool isInKnockback() const { return _isKnockedBack; }

protected:
    Player* _targetPlayer;
    int _attackDamage;
    float _attackRange;
    int _expReward;
    int _hitCount;
    int _hitsToDie;

    virtual int getHitsToDieForPlayer(Player* player) const;

    float _attackCooldown;
    float _attackCooldownMax;

    cocos2d::Action* _idleAction;
    float _idleBaseScale;
    bool _isAttacking;
    float _telegraphTimer;

    // Projectile system
    std::vector<EnemyProjectile> _projectiles;
    float _projectileCooldown;
    float _projectileCooldownMax;

    // Knockback state
    bool _isKnockedBack = false;
    cocos2d::Vec2 _knockbackVelocity;
    cocos2d::Vec2 _knockbackDir;       // original knockback direction for CD detection
    float _knockbackAccelMag;          // |a| = 4×speed
};

#endif
