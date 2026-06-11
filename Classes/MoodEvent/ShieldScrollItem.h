#pragma once
#include "GroundItem.h"

class ShieldScrollItem : public GroundItem
{
public:
    static ShieldScrollItem* create(const cocos2d::Vec2& pos);

    virtual void applyToPlayer(Player* player) override;
};