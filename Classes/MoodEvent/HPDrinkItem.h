#pragma once
#include "GroundItem.h"

class HPDrinkItem : public GroundItem
{
public:
    static HPDrinkItem* create(const cocos2d::Vec2& pos, int healAmount, float focusDuration = 2.0f);

    virtual void applyToPlayer(Player* player) override;

private:
    int _healAmount = 20;
    float _focusDuration = 2.0f;
};