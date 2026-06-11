#include "GroundItemManager.h"
#include <algorithm>

USING_NS_CC;

GroundItemManager* GroundItemManager::create(Player* player, cocos2d::Node* parentLayer)
{
    auto mgr = new (std::nothrow) GroundItemManager();
    if (mgr && mgr->init(player, parentLayer))
    {
        mgr->autorelease();
        return mgr;
    }
    CC_SAFE_DELETE(mgr);
    return nullptr;
}

bool GroundItemManager::init(Player* player, cocos2d::Node* parentLayer)
{
    _player = player;
    _parentLayer = parentLayer;
    return true;
}

void GroundItemManager::spawnCoffee(const Vec2& pos)
{
    auto item = CoffeeBuffItem::create(pos, 3.0f);
    if (!item) return;
    _parentLayer->addChild(item);
    _items.push_back(item);
}

void GroundItemManager::spawnHPDrink(const Vec2& pos)
{
    auto item = HPDrinkItem::create(pos, 25, 2.0f);
    if (!item) return;
    _parentLayer->addChild(item);
    _items.push_back(item);
}

void GroundItemManager::spawnShield(const Vec2& pos)
{
    auto item = ShieldScrollItem::create(pos);
    if (!item) return;
    _parentLayer->addChild(item);
    _items.push_back(item);
}

void GroundItemManager::update(float /*dt*/)
{
    if (!_player || !_parentLayer) return;

    auto pBox = _player->getCollisionBox();

    for (auto& item : _items)
    {
        if (!item) continue;
        if (!item->isObjectActive()) continue;

        if (pBox.intersectsRect(item->getCollisionBox()))
        {
            item->onPicked(_player);
        }
    }

    // 清理无效指针（保守清理）
    _items.erase(
        std::remove_if(_items.begin(), _items.end(),
            [](GroundItem* it)
            {
                return (it == nullptr);
            }),
        _items.end()
    );
}