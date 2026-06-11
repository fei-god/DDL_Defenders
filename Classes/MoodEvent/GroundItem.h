#pragma once

#include "cocos2d.h"
#include "GameObject.h"
#include "Player.h"

class GroundItem : public GameObject
{
public:
    virtual ~GroundItem() {}

    // 地上拾取：当玩家碰到后执行
    virtual void applyToPlayer(Player* player) = 0;

    // 你可以让道具在拾取后失效并移除
    void onPicked(Player* player);
};