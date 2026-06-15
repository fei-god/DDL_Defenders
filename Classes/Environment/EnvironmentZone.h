#pragma once
#ifndef __ENVIRONMENT_ZONE_H__
#define __ENVIRONMENT_ZONE_H__

#include "GameObject.h"

enum class EnvironmentZoneType
{
    Bed,
    Desk,
    PowerSocket,
    CoffeeArea
};

class EnvironmentZone : public GameObject
{
public:
    static EnvironmentZone* createZone(EnvironmentZoneType type,
        const cocos2d::Rect& bounds);

    bool initZone(EnvironmentZoneType type, const cocos2d::Rect& bounds);

    EnvironmentZoneType getZoneType() const;
    std::string getEffectText() const;
    virtual cocos2d::Rect getCollisionBox() const override;

private:
    EnvironmentZoneType _zoneType;
    cocos2d::Rect _bounds;
};

#endif
