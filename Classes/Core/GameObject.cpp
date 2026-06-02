#include "GameObject.h"

USING_NS_CC;

GameObject::GameObject()
    : objectName("")
    , objectType(GameObjectType::Unknown)
    , imagePath("")
    , isActive(true)
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
    // GameObject 只提供基础更新接口
    // 具体逻辑由 Role、Player、Enemy、Weapon 等子类重写
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

    return true;
}

std::string GameObject::getImagePath() const
{
    return imagePath;
}

void GameObject::setObjectPosition(const Vec2& position)
{
    this->setPosition(position);
}

Vec2 GameObject::getObjectPosition() const
{
    return this->getPosition();
}

Rect GameObject::getCollisionBox() const
{
    return this->getBoundingBox();
}

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
