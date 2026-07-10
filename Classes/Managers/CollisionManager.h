#pragma once

#include "Enemy.h"
#include "Bullet.h"
#include <vector>

class CollisionManager
{
public:
    static void checkBulletEnemyCollision(
        std::vector<Bullet*>& bullets,
        std::vector<Enemy*>& enemies
    );

    static void clearInactiveBullets(std::vector<Bullet*>& bullets);
};
