#pragma once
#include "GroundItem.h"
#include "MoodSystem.h"

class CoffeeBuffItem : public GroundItem
{
public:
    static CoffeeBuffItem* create(const cocos2d::Vec2& pos, float focusDuration);

    virtual void applyToPlayer(Player* player) override;

private:
    float _focusDuration = 3.0f;
};