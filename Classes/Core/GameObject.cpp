#include "GameObject.h"

#include <algorithm>
#include <sstream>
#include <cmath>

USING_NS_CC;

int GameObject::nextObjectId = 1;

namespace
{
    constexpr unsigned char COLLISION_ALPHA_THRESHOLD = 8;

    bool isVisiblePixel(const unsigned char* data,
        ssize_t dataLen,
        int index,
        cocos2d::backend::PixelFormat format)
    {
        if (data == nullptr || index < 0)
        {
            return false;
        }

        switch (format)
        {
        case cocos2d::backend::PixelFormat::RGBA8888:
        case cocos2d::backend::PixelFormat::BGRA8888:
        {
            ssize_t offset = static_cast<ssize_t>(index) * 4 + 3;
            return offset < dataLen && data[offset] > COLLISION_ALPHA_THRESHOLD;
        }
        case cocos2d::backend::PixelFormat::A8:
        {
            ssize_t offset = static_cast<ssize_t>(index);
            return offset < dataLen && data[offset] > COLLISION_ALPHA_THRESHOLD;
        }
        case cocos2d::backend::PixelFormat::AI88:
        {
            ssize_t offset = static_cast<ssize_t>(index) * 2 + 1;
            return offset < dataLen && data[offset] > COLLISION_ALPHA_THRESHOLD;
        }
        case cocos2d::backend::PixelFormat::RGB5A1:
        {
            ssize_t offset = static_cast<ssize_t>(index) * 2;
            if (offset + 1 >= dataLen) return false;
            unsigned short pixel = static_cast<unsigned short>(data[offset])
                | (static_cast<unsigned short>(data[offset + 1]) << 8);
            return (pixel & 0x0001) != 0;
        }
        case cocos2d::backend::PixelFormat::RGBA4444:
        {
            ssize_t offset = static_cast<ssize_t>(index) * 2;
            if (offset + 1 >= dataLen) return false;
            unsigned short pixel = static_cast<unsigned short>(data[offset])
                | (static_cast<unsigned short>(data[offset + 1]) << 8);
            unsigned char alpha = static_cast<unsigned char>((pixel & 0x000F) * 17);
            return alpha > COLLISION_ALPHA_THRESHOLD;
        }
        default:
            return true;
        }
    }

    Rect calculateAlphaCollisionBox(const std::string& imagePath, const Size& textureSize)
    {
        if (imagePath.empty() || textureSize.width <= 0.0f || textureSize.height <= 0.0f)
        {
            return Rect(0, 0, textureSize.width, textureSize.height);
        }

        Image image;
        if (!image.initWithImageFile(imagePath) || !image.hasAlpha())
        {
            return Rect(0, 0, textureSize.width, textureSize.height);
        }

        unsigned char* data = image.getData();
        int width = image.getWidth();
        int height = image.getHeight();
        if (data == nullptr || width <= 0 || height <= 0)
        {
            return Rect(0, 0, textureSize.width, textureSize.height);
        }

        int minX = width;
        int minY = height;
        int maxX = -1;
        int maxY = -1;
        auto format = image.getPixelFormat();
        ssize_t dataLen = image.getDataLen();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = y * width + x;
                if (!isVisiblePixel(data, dataLen, index, format))
                {
                    continue;
                }

                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }

        if (maxX < minX || maxY < minY)
        {
            return Rect(0, 0, textureSize.width, textureSize.height);
        }

        float scaleX = textureSize.width / static_cast<float>(width);
        float scaleY = textureSize.height / static_cast<float>(height);
        float localX = static_cast<float>(minX) * scaleX;
        float localY = static_cast<float>(height - 1 - maxY) * scaleY;
        float localW = static_cast<float>(maxX - minX + 1) * scaleX;
        float localH = static_cast<float>(maxY - minY + 1) * scaleY;

        return Rect(localX, localY, localW, localH);
    }
}

GameObject::GameObject()
    : objectId(nextObjectId++)
    , objectName("")
    , objectType(GameObjectType::Unknown)
    , objectCamp(GameObjectCamp::Neutral)
    , imagePath("")
    , isActive(true)
    , updateEnabled(true)
    , collisionEnabled(true)
    , shouldDestroy(false)
    , velocity(Vec2::ZERO)
    , direction(Vec2(1.0f, 0.0f))
    , moveSpeed(0.0f)
    , collisionRadius(20.0f)
    , useCircleCollision(true)
    , collisionLocalBox(Rect::ZERO)
    , hasCollisionLocalBox(false)
    , lifeTime(0.0f)
    , currentLifeTime(0.0f)
    , hasLifeTimeLimit(false)
{
}

GameObject::~GameObject()
{
}

bool GameObject::init()
{
    if (!Sprite::init())
    {
        return false;
    }

    isActive = true;
    updateEnabled = true;
    collisionEnabled = true;
    shouldDestroy = false;
    currentLifeTime = 0.0f;

    return true;
}

bool GameObject::initObject(
    const std::string& name,
    GameObjectType type,
    const std::string& imagePath,
    const Vec2& startPosition
)
{
    if (!Sprite::init())
    {
        return false;
    }

    this->objectName = name;
    this->objectType = type;
    this->imagePath = imagePath;
    this->isActive = true;
    this->updateEnabled = true;
    this->collisionEnabled = true;
    this->shouldDestroy = false;
    this->currentLifeTime = 0.0f;

    // ���ݶ�����������Ĭ����Ӫ
    if (type == GameObjectType::Player)
    {
        this->objectCamp = GameObjectCamp::Player;
    }
    else if (type == GameObjectType::Enemy)
    {
        this->objectCamp = GameObjectCamp::Enemy;
    }
    else
    {
        this->objectCamp = GameObjectCamp::Neutral;
    }

    if (!imagePath.empty())
    {
        if (!setObjectImage(imagePath))
        {
            return false;
        }
    }

    setPosition(startPosition);

    return true;
}

GameObject* GameObject::create(
    const std::string& name,
    GameObjectType type,
    const std::string& imagePath,
    const Vec2& startPosition
)
{
    GameObject* object = new GameObject();

    if (object && object->initObject(name, type, imagePath, startPosition))
    {
        object->autorelease();
        return object;
    }

    CC_SAFE_DELETE(object);
    return nullptr;
}

void GameObject::updateObject(float dt)
{
    if (!isActive || !updateEnabled)
    {
        return;
    }

    updateLifeTime(dt);

    if (isExpired())
    {
        markForDestroy();
        return;
    }

    moveByVelocity(dt);
}

// =========================
// ����������Ϣ
// =========================

int GameObject::getObjectId() const
{
    return objectId;
}

void GameObject::setObjectName(const std::string& name)
{
    this->objectName = name;
}

std::string GameObject::getObjectName() const
{
    return objectName;
}

void GameObject::setObjectType(GameObjectType type)
{
    this->objectType = type;
}

GameObjectType GameObject::getObjectType() const
{
    return objectType;
}

void GameObject::setObjectCamp(GameObjectCamp camp)
{
    this->objectCamp = camp;
}

GameObjectCamp GameObject::getObjectCamp() const
{
    return objectCamp;
}

bool GameObject::isSameCampWith(const GameObject* other) const
{
    if (other == nullptr)
    {
        return false;
    }

    return this->objectCamp == other->getObjectCamp();
}

bool GameObject::isEnemyCampWith(const GameObject* other) const
{
    if (other == nullptr)
    {
        return false;
    }

    if (this->objectCamp == GameObjectCamp::Neutral ||
        other->getObjectCamp() == GameObjectCamp::Neutral)
    {
        return false;
    }

    return this->objectCamp != other->getObjectCamp();
}

// =========================
// ��ǩϵͳ
// =========================

void GameObject::addTag(const std::string& tag)
{
    if (!hasTag(tag))
    {
        tags.push_back(tag);
    }
}

void GameObject::removeTag(const std::string& tag)
{
    auto iter = std::remove(tags.begin(), tags.end(), tag);
    tags.erase(iter, tags.end());
}

bool GameObject::hasTag(const std::string& tag) const
{
    return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

void GameObject::clearTags()
{
    tags.clear();
}

// =========================
// ͼƬ����ʾ
// =========================

bool GameObject::setObjectImage(const std::string& imagePath)
{
    auto texture = Director::getInstance()
        ->getTextureCache()
        ->addImage(imagePath);

    if (texture == nullptr)
    {
        return false;
    }

    this->imagePath = imagePath;
    this->setTexture(texture);

    Size textureSize = texture->getContentSize();
    this->setTextureRect(Rect(0, 0, textureSize.width, textureSize.height));
    setCollisionLocalBox(calculateAlphaCollisionBox(imagePath, textureSize));

    Rect localBox = getCollisionLocalBox();
    if (localBox.size.width > 0 && localBox.size.height > 0)
    {
        collisionRadius = std::min(localBox.size.width, localBox.size.height) * 0.5f;
    }

    return true;
}

std::string GameObject::getImagePath() const
{
    return imagePath;
}

void GameObject::setObjectScale(float scale)
{
    this->setScale(scale);
}

float GameObject::getObjectScale() const
{
    return this->getScale();
}

// =========================
// λ�á������ƶ�
// =========================

void GameObject::setObjectPosition(const Vec2& position)
{
    this->setPosition(position);
}

Vec2 GameObject::getObjectPosition() const
{
    return this->getPosition();
}

void GameObject::setVelocity(const Vec2& velocity)
{
    this->velocity = velocity;

    if (!velocity.isZero())
    {
        Vec2 normalized = velocity;
        normalized.normalize();
        this->direction = normalized;
    }
}

Vec2 GameObject::getVelocity() const
{
    return velocity;
}

void GameObject::setDirection(const Vec2& direction)
{
    if (direction.isZero())
    {
        return;
    }

    Vec2 normalized = direction;
    normalized.normalize();
    this->direction = normalized;
}

Vec2 GameObject::getDirection() const
{
    return direction;
}

void GameObject::setMoveSpeed(float speed)
{
    this->moveSpeed = std::max(0.0f, speed);
}

float GameObject::getMoveSpeed() const
{
    return moveSpeed;
}

void GameObject::moveByVelocity(float dt)
{
    if (!isActive || !updateEnabled)
    {
        return;
    }

    if (velocity.isZero())
    {
        return;
    }

    this->setPosition(this->getPosition() + velocity * dt);
}


// =========================
// ��ײ�뷶Χ�ж�
// =========================

Rect GameObject::getCollisionBox() const
{
    if (!hasCollisionLocalBox)
    {
        return this->getBoundingBox();
    }

    return RectApplyAffineTransform(collisionLocalBox, this->getNodeToParentAffineTransform());
}

void GameObject::setCollisionLocalBox(const Rect& box)
{
    if (box.size.width <= 0.0f || box.size.height <= 0.0f)
    {
        clearCollisionLocalBox();
        return;
    }

    collisionLocalBox = box;
    hasCollisionLocalBox = true;
}

void GameObject::clearCollisionLocalBox()
{
    collisionLocalBox = Rect::ZERO;
    hasCollisionLocalBox = false;
}

Rect GameObject::getCollisionLocalBox() const
{
    if (hasCollisionLocalBox)
    {
        return collisionLocalBox;
    }

    Size size = getContentSize();
    return Rect(0, 0, size.width, size.height);
}

void GameObject::setCollisionRadius(float radius)
{
    this->collisionRadius = std::max(0.0f, radius);
}

float GameObject::getCollisionRadius() const
{
    // Use average scale to support non-uniformly scaled objects (e.g. laser beams)
    return collisionRadius * (this->getScaleX() + this->getScaleY()) * 0.5f;
}


bool GameObject::isUsingCircleCollision() const
{
    return useCircleCollision;
}

void GameObject::setCollisionEnabled(bool enabled)
{
    this->collisionEnabled = enabled;
}

bool GameObject::isCollisionEnabled() const
{
    return collisionEnabled;
}

bool GameObject::isCollidingWith(const GameObject* other) const
{
    if (other == nullptr)
    {
        return false;
    }

    if (!this->isActive || !other->isObjectActive())
    {
        return false;
    }

    if (!this->collisionEnabled || !other->isCollisionEnabled())
    {
        return false;
    }

    if (this->useCircleCollision || other->isUsingCircleCollision())
    {
        float distance = this->distanceTo(other);
        float radiusSum = this->getCollisionRadius() + other->getCollisionRadius();
        return distance <= radiusSum;
    }

    return this->getCollisionBox().intersectsRect(other->getCollisionBox());
}

bool GameObject::isInRangeOf(const GameObject* other, float range) const
{
    if (other == nullptr)
    {
        return false;
    }

    return distanceTo(other) <= range;
}

float GameObject::distanceTo(const GameObject* other) const
{
    if (other == nullptr)
    {
        return 999999.0f;
    }

    return this->getPosition().distance(other->getPosition());
}

float GameObject::distanceToPoint(const Vec2& point) const
{
    return this->getPosition().distance(point);
}

// =========================
// ��������
// =========================

void GameObject::setLifeTime(float time)
{
    this->lifeTime = std::max(0.0f, time);
    this->currentLifeTime = 0.0f;
    this->hasLifeTimeLimit = time > 0.0f;
}

float GameObject::getLifeTime() const
{
    return lifeTime;
}



void GameObject::updateLifeTime(float dt)
{
    if (!hasLifeTimeLimit)
    {
        return;
    }

    currentLifeTime += dt;
}

bool GameObject::isExpired() const
{
    if (!hasLifeTimeLimit)
    {
        return false;
    }

    return currentLifeTime >= lifeTime;
}

// =========================
// ������¡�����������
// =========================

void GameObject::setActive(bool active)
{
    this->isActive = active;

    this->setVisible(active);

    if (!active)
    {
        this->pause();
    }
    else
    {
        this->resume();
    }
}

bool GameObject::isObjectActive() const
{
    return isActive;
}

void GameObject::setUpdateEnabled(bool enabled)
{
    this->updateEnabled = enabled;
}

bool GameObject::isUpdateEnabled() const
{
    return updateEnabled;
}



void GameObject::markForDestroy()
{
    shouldDestroy = true;
    isActive = false;
    collisionEnabled = false;
    updateEnabled = false;
    setVisible(false);
}

bool GameObject::isMarkedForDestroy() const
{
    return shouldDestroy;
}

// =========================
// ��ͼ�߽�
// =========================

bool GameObject::isOutOfBounds(const Rect& bounds) const
{
    Vec2 pos = this->getPosition();

    return pos.x < bounds.getMinX() ||
        pos.x > bounds.getMaxX() ||
        pos.y < bounds.getMinY() ||
        pos.y > bounds.getMaxY();
}


// =========================
// ������Ϣ
// =========================

std::string GameObject::getDebugInfo() const
{
    std::ostringstream oss;

    oss << "[GameObject]"
        << " id=" << objectId
        << " name=" << objectName
        << " active=" << (isActive ? "true" : "false")
        << " pos=(" << getPositionX() << ", " << getPositionY() << ")"
        << " speed=" << moveSpeed
        << " radius=" << collisionRadius;

    return oss.str();
}
