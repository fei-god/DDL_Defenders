#pragma once
#ifndef __BULLET_POOL_H__
#define __BULLET_POOL_H__

#include "Bullet.h"
#include <vector>

class BulletPool
{
public:
    BulletPool();
    ~BulletPool();

    void init(cocos2d::Node* bulletLayer, int initialCount);

    Bullet* acquire(const std::string& name,
        const std::string& imagePath,
        const cocos2d::Vec2& startPosition,
        const cocos2d::Vec2& direction,
        float speed,
        int damage,
        float lifeTime,
        bool canPierce);

    void reclaimInactive();

private:
    cocos2d::Node* _bulletLayer;
    std::vector<Bullet*> _available;
    std::vector<Bullet*> _active;
};

#endif
