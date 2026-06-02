#include "Bullet.h"
#include <algorithm>
#include <new>

USING_NS_CC;

Bullet::Bullet()
    : _direction(Vec2::ZERO)
    , _speed(0.0f)
    , _damage(0)
    , _lifeTime(0.0f)
    , _timer(0.0f)
    , _canPierce(false)
    , _expired(false)
{
}

Bullet::~Bullet()
{
}

Bullet* Bullet::createBullet(
    const std::string& name,
    const std::string& imagePath,
    const Vec2& startPosition,
    const Vec2& direction,
    float speed,
    int damage,
    float lifeTime,
    bool canPierce
)
{
    Bullet* bullet = new (std::nothrow) Bullet();

    if (bullet && bullet->initBullet(
        name,
        imagePath,
        startPosition,
        direction,
        speed,
        damage,
        lifeTime,
        canPierce
    ))
    {
        bullet->autorelease();
        return bullet;
    }

    CC_SAFE_DELETE(bullet);
    return nullptr;
}

bool Bullet::initBullet(
    const std::string& name,
    const std::string& imagePath,
    const Vec2& startPosition,
    const Vec2& direction,
    float speed,
    int damage,
    float lifeTime,
    bool canPierce
)
{

    // 先用空图片初始化子弹对象，保证子弹对象一定能创建出来。
    // 后面再尝试加载图片。这样即使图片不存在，也不影响子弹移动和碰撞测试。
    if (!GameObject::initObject(
        name,
        GameObjectType::Bullet,
        "",
        startPosition
    ))
    {
        return false;
    }

    // 有图片就加载图片；图片不存在就给一个18×18的碰撞矩形兜底。
    if (!imagePath.empty())
    {
        if (!setObjectImage(imagePath))
        {
            setTextureRect(Rect(0, 0, 18, 18));
        }
    }
    else
    {
        setTextureRect(Rect(0, 0, 18, 18));
    }


    if (direction.lengthSquared() < 0.0001f)
    {
        _direction = Vec2(1, 0);
    }
    else
    {
        _direction = direction.getNormalized();
    }

    _speed = speed;
    _damage = damage;
    _lifeTime = lifeTime;
    _timer = 0.0f;
    _canPierce = canPierce;
    _expired = false;

    setRotation(-CC_RADIANS_TO_DEGREES(_direction.getAngle()));

    return true;
}

void Bullet::updateObject(float dt)
{
    if (!isObjectActive() || _expired)
    {
        return;
    }

    // 位移 = 方向 × 速度 × 时间
    Vec2 newPosition = getPosition() + _direction * _speed * dt;
    setPosition(newPosition);

    _timer += dt;

    if (_timer >= _lifeTime)
    {
        _expired = true;
        setActive(false);
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    if (newPosition.x < -100 ||
        newPosition.y < -100 ||
        newPosition.x > visibleSize.width + 100 ||
        newPosition.y > visibleSize.height + 100)
    {
        _expired = true;
        setActive(false);
    }
}

Rect Bullet::getCollisionBox() const
{
    return GameObject::getCollisionBox();
}

int Bullet::getDamage() const
{
    return _damage;
}

bool Bullet::isExpired() const
{
    return _expired || !isObjectActive();
}

bool Bullet::isPiercing() const
{
    return _canPierce;
}

void Bullet::markHit()
{
    // 非穿透子弹命中后失效；穿透子弹命中后继续飞。
    if (!_canPierce)
    {
        _expired = true;
        setActive(false);
    }
}

bool Bullet::hasHitObject(GameObject* object) const
{
    return std::find(_hitObjects.begin(), _hitObjects.end(), object) != _hitObjects.end();
}

void Bullet::recordHitObject(GameObject* object)
{
    if (object == nullptr)
    {
        return;
    }

    if (!hasHitObject(object))
    {
        _hitObjects.push_back(object);
    }
}