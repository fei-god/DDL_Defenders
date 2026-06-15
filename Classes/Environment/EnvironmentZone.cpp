#include "EnvironmentZone.h"
#include "Core/AssetPaths.h"
#include "platform/CCFileUtils.h"

USING_NS_CC;

EnvironmentZone* EnvironmentZone::createZone(EnvironmentZoneType type,
    const Rect& bounds)
{
    EnvironmentZone* zone = new (std::nothrow) EnvironmentZone();
    if (zone && zone->initZone(type, bounds))
    {
        zone->autorelease();
        return zone;
    }
    CC_SAFE_DELETE(zone);
    return nullptr;
}

bool EnvironmentZone::initZone(EnvironmentZoneType type, const Rect& bounds)
{
    if (!GameObject::initObject("EnvironmentZone", GameObjectType::Environment, "",
        Vec2(bounds.getMidX(), bounds.getMidY())))
    {
        return false;
    }

    _zoneType = type;
    _bounds = bounds;
    setContentSize(bounds.size);
    setAnchorPoint(Vec2(0.5f, 0.5f));

    Color4F color;
    std::string labelText;
    std::string imagePath;
    switch (_zoneType)
    {
    case EnvironmentZoneType::Bed:
        setObjectName("Bed");
        color = Color4F(0.30f, 0.42f, 0.78f, 0.28f);
        labelText = "Bed";
        imagePath = "art/props/bed.png";
        break;
    case EnvironmentZoneType::Desk:
        setObjectName("Desk");
        color = Color4F(0.28f, 0.70f, 0.40f, 0.28f);
        labelText = "Desk";
        imagePath = "art/props/desk.png";
        break;
    case EnvironmentZoneType::PowerSocket:
        setObjectName("PowerSocket");
        color = Color4F(0.92f, 0.82f, 0.25f, 0.28f);
        labelText = "Socket";
        imagePath = "art/props/power_socket.png";
        break;
    case EnvironmentZoneType::CoffeeArea:
    default:
        setObjectName("CoffeeArea");
        color = Color4F(0.75f, 0.34f, 0.20f, 0.28f);
        labelText = "Coffee";
        imagePath = "art/props/coffee_area.png";
        break;
    }

    imagePath = AssetPaths::resolve(imagePath);
    if (!imagePath.empty())
    {
        auto sprite = Sprite::create(imagePath);
        if (sprite)
        {
            Size imageSize = sprite->getContentSize();
            if (imageSize.width > 0.0f && imageSize.height > 0.0f)
            {
                sprite->setScale(std::min(bounds.size.width / imageSize.width,
                    bounds.size.height / imageSize.height));
            }
            addChild(sprite);
        }
    }
    else
    {
        auto draw = DrawNode::create();
        Vec2 verts[4] = {
            Vec2(-bounds.size.width * 0.5f, -bounds.size.height * 0.5f),
            Vec2(bounds.size.width * 0.5f, -bounds.size.height * 0.5f),
            Vec2(bounds.size.width * 0.5f, bounds.size.height * 0.5f),
            Vec2(-bounds.size.width * 0.5f, bounds.size.height * 0.5f)
        };
        draw->drawPolygon(verts, 4, color, 1.0f, Color4F(color.r, color.g, color.b, 0.8f));
        addChild(draw);
    }

    auto label = Label::createWithSystemFont(labelText, "Arial", 16);
    label->setColor(Color3B(230, 235, 240));
    label->setPosition(Vec2::ZERO);
    addChild(label);

    return true;
}

EnvironmentZoneType EnvironmentZone::getZoneType() const
{
    return _zoneType;
}

std::string EnvironmentZone::getEffectText() const
{
    switch (_zoneType)
    {
    case EnvironmentZoneType::Bed:
        return "Near Bed: Exhausted +";
    case EnvironmentZoneType::Desk:
        return "Near Desk: Focus +";
    case EnvironmentZoneType::PowerSocket:
        return "Near Socket: Laser +";
    case EnvironmentZoneType::CoffeeArea:
        return "Near Coffee: Excited +";
    default:
        return "";
    }
}

Rect EnvironmentZone::getCollisionBox() const
{
    return _bounds;
}
