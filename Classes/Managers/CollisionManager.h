#pragma once

#include "Enemy.h"
#include "Bullet.h"
#include <vector>

class CollisionManager
{
public:
    // 检测子弹和敌人是否碰撞。
    static void checkBulletEnemyCollision(
        std::vector<Bullet*>& bullets,
        std::vector<Enemy*>& enemies
    );

    // 清理已经失效的子弹。
    static void clearInactiveBullets(std::vector<Bullet*>& bullets);
};