#include "BulletPool.h"

USING_NS_CC;

BulletPool::BulletPool()
    : _bulletLayer(nullptr)
{
}

BulletPool::~BulletPool()
{
    for (auto* bullet : _available)
    {
        CC_SAFE_RELEASE(bullet);
    }
    for (auto* bullet : _active)
    {
        CC_SAFE_RELEASE(bullet);
    }
}

void BulletPool::init(Node* bulletLayer, int initialCount)
{
    _bulletLayer = bulletLayer;
    for (int i = 0; i < initialCount; ++i)
    {
        auto* bullet = Bullet::createBullet("PooledBullet", "", Vec2::ZERO,
            Vec2(1, 0), 0.0f, 0, 0.1f, false);
        if (bullet)
        {
            bullet->retain();
            bullet->setActive(false);
            bullet->setVisible(false);
            _available.push_back(bullet);
        }
    }
}

Bullet* BulletPool::acquire(const std::string& name,
    const std::string& imagePath,
    const Vec2& startPosition,
    const Vec2& direction,
    float speed,
    int damage,
    float lifeTime,
    bool canPierce)
{
    Bullet* bullet = nullptr;
    if (!_available.empty())
    {
        bullet = _available.back();
        _available.pop_back();
        bullet->resetBullet(name, imagePath, startPosition, direction, speed,
            damage, lifeTime, canPierce);
    }
    else
    {
        bullet = Bullet::createBullet(name, imagePath, startPosition, direction,
            speed, damage, lifeTime, canPierce);
        if (bullet)
        {
            bullet->retain();
        }
    }

    if (bullet && _bulletLayer)
    {
        if (bullet->getParent() == nullptr)
        {
            _bulletLayer->addChild(bullet, 30);
        }
        bullet->setLocalZOrder(30);
        bullet->setVisible(true);
        _active.push_back(bullet);
    }

    return bullet;
}

void BulletPool::reclaimInactive()
{
    for (auto it = _active.begin(); it != _active.end(); )
    {
        Bullet* bullet = *it;
        if (bullet == nullptr || bullet->isExpired() || !bullet->isObjectActive())
        {
            if (bullet)
            {
                bullet->removeFromParent();
                bullet->setVisible(false);
                _available.push_back(bullet);
            }
            it = _active.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
