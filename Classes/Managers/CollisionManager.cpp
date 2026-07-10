#include "CollisionManager.h"
#include "DamageCalculator.h"
#include "AudioManager.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr float COLLISION_GRID_CELL_SIZE = 128.0f;

    long long gridKey(int x, int y)
    {
        return (static_cast<long long>(x) << 32)
            ^ static_cast<unsigned int>(y);
    }

    int cellIndex(float value)
    {
        return static_cast<int>(std::floor(value / COLLISION_GRID_CELL_SIZE));
    }

    bool isEnemyCollisionCandidate(Enemy* enemy)
    {
        return enemy != nullptr &&
            enemy->isObjectActive() &&
            !enemy->isDead();
    }
}

void CollisionManager::checkBulletEnemyCollision(
    std::vector<Bullet*>& bullets,
    std::vector<Enemy*>& enemies
)
{
    std::unordered_map<long long, std::vector<Enemy*>> enemyGrid;
    enemyGrid.reserve(enemies.size() * 2 + 1);

    for (Enemy* enemy : enemies)
    {
        if (!isEnemyCollisionCandidate(enemy))
        {
            continue;
        }

        cocos2d::Rect box = enemy->getCollisionBox();
        int minCellX = cellIndex(box.getMinX());
        int maxCellX = cellIndex(box.getMaxX());
        int minCellY = cellIndex(box.getMinY());
        int maxCellY = cellIndex(box.getMaxY());

        for (int y = minCellY; y <= maxCellY; ++y)
        {
            for (int x = minCellX; x <= maxCellX; ++x)
            {
                enemyGrid[gridKey(x, y)].push_back(enemy);
            }
        }
    }

    for (Bullet* bullet : bullets)
    {
        if (bullet == nullptr)
        {
            continue;
        }

        if (!bullet->isObjectActive() || bullet->isExpired())
        {
            continue;
        }

        cocos2d::Rect bulletBox = bullet->getCollisionBox();
        int minCellX = cellIndex(bulletBox.getMinX());
        int maxCellX = cellIndex(bulletBox.getMaxX());
        int minCellY = cellIndex(bulletBox.getMinY());
        int maxCellY = cellIndex(bulletBox.getMaxY());
        std::vector<Enemy*> candidates;

        for (int y = minCellY; y <= maxCellY; ++y)
        {
            for (int x = minCellX; x <= maxCellX; ++x)
            {
                auto found = enemyGrid.find(gridKey(x, y));
                if (found == enemyGrid.end())
                {
                    continue;
                }

                for (Enemy* enemy : found->second)
                {
                    if (std::find(candidates.begin(), candidates.end(), enemy) == candidates.end())
                    {
                        candidates.push_back(enemy);
                    }
                }
            }
        }

        for (Enemy* enemy : candidates)
        {
            if (!isEnemyCollisionCandidate(enemy))
            {
                continue;
            }

            if (bullet->hasHitObject(enemy))
            {
                continue;
            }

            if (!bulletBox.intersectsRect(enemy->getCollisionBox()))
            {
                continue;
            }

            int finalDamage = DamageCalculator::calculateFinalDamage(
                bullet->getDamage(),
                0,
                1.0f
            );

            bool wasAlive = enemy->isRoleAlive();
            enemy->takeDamage(finalDamage);
            if (wasAlive)
            {
                if (enemy->isDead()) AudioManager::getInstance()->playEnemyDie();
                else AudioManager::getInstance()->playEnemyHit();
            }

            bullet->recordHitObject(enemy);
            bullet->markHit();

            if (enemy->isDead())
            {
                enemy->setActive(false);
            }

            if (!bullet->isPiercing())
            {
                break;
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
