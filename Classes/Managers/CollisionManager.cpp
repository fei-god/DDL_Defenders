#include "CollisionManager.h"
#include "DamageCalculator.h"
#include <algorithm>

void CollisionManager::checkBulletEnemyCollision(
    std::vector<Bullet*>& bullets,
    std::vector<Enemy*>& enemies
)
{
    // 遍历当前场景中的所有子弹
    for (Bullet* bullet : bullets)
    {
        if (bullet == nullptr)
        {
            continue;
        }

        // 如果子弹已经失效，就不再参与碰撞检测
        if (!bullet->isObjectActive() || bullet->isExpired())
        {
            continue;
        }

        // 一颗子弹需要依次检测它是否打中了某个敌人
        for (Enemy* enemy : enemies)
        {
            if (enemy == nullptr)
            {
                continue;
            }

            // 非激活敌人不参与碰撞检测
            if (!enemy->isObjectActive())
            {
                continue;
            }

            // 已死亡敌人不参与碰撞检测
            if (enemy->isDead())
            {
                continue;
            }

            // CoffeeLaser是穿透子弹，可能连续多帧和同一个敌人重叠。
            // 如果不记录已命中的敌人，就会一帧扣一次血，伤害会异常高。
            if (bullet->hasHitObject(enemy))
            {
                continue;
            }

            // getCollisionBox()返回对象的矩形碰撞盒。
            // intersectsRect()判断两个矩形是否有重叠。
            bool isCollide = bullet->getCollisionBox().intersectsRect(enemy->getCollisionBox());

            if (isCollide)
            {
                // 这里defense传0，是因为Role::takeDamage()里面已经会扣一次防御。
                // 如果这里再传enemy->getDefense()，就会出现防御被扣两次的问题。
                int finalDamage = DamageCalculator::calculateFinalDamage(
                    bullet->getDamage(),
                    0,
                    1.0f
                );

                // 敌人受到伤害
                enemy->takeDamage(finalDamage);

                // 记录这个子弹已经打中过这个敌人
                bullet->recordHitObject(enemy);

                // 普通子弹命中后失效，穿透子弹不会失效
                bullet->markHit();

                // 如果敌人死亡，设置为非激活状态
                // Role::takeDamage()内部已经会调用die()，这里不需要再手动die()
                if (enemy->isDead())
                {
                    enemy->setActive(false);
                }

                // 如果不是穿透子弹，命中一个敌人后就停止检测其他敌人
                if (!bullet->isPiercing())
                {
                    break;
                }
            }
        }
    }
}

void CollisionManager::clearInactiveBullets(std::vector<Bullet*>& bullets)
{
    bullets.erase(
        std::remove_if(
            bullets.begin(),
            bullets.end(),
            [](Bullet* bullet)
            {
                if (bullet == nullptr)
                {
                    return true;
                }

                // 子弹命中、超时、飞出屏幕后，会变成expired或者inactive。
                // 这些子弹必须从父节点中移除，否则场景里的对象会越来越多。
                if (bullet->isExpired() || !bullet->isObjectActive())
                {
                    bullet->removeFromParent();
                    return true;
                }

                return false;
            }
        ),
        bullets.end()
    );
}