#pragma once
#include "cocos2d.h"
#include "Player.h"
#include "CoffeeBuffItem.h"
#include "HPDrinkItem.h"
#include "ShieldScrollItem.h"

class GroundItemManager : public cocos2d::Ref
{
public:
    static GroundItemManager* create(Player* player, cocos2d::Node* parentLayer);

    bool init(Player* player, cocos2d::Node* parentLayer);

    void update(float dt); // 场景每帧调用
    void spawnCoffee(const cocos2d::Vec2& pos);
    void spawnHPDrink(const cocos2d::Vec2& pos);
    void spawnShield(const cocos2d::Vec2& pos);

private:
    Player* _player = nullptr;
    cocos2d::Node* _parentLayer = nullptr;

    std::vector<GroundItem*> _items;
};