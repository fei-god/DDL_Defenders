#include "CoffeeBuffItem.h"

USING_NS_CC;

CoffeeBuffItem* CoffeeBuffItem::create(const Vec2& pos, float focusDuration)
{
    auto item = new (std::nothrow) CoffeeBuffItem();
    if (item && item->initObject("coffee", GameObjectType::Item, "item_coffee.png", pos))
    {
        item->_focusDuration = focusDuration;
        item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return nullptr;
}

void CoffeeBuffItem::applyToPlayer(Player* player)
{
    // 增加移动速度/短时提升 Focus
    if (!player) return;
    player->changeMood(MoodType::Focus, _focusDuration);
    // 你也可以顺手给个“战斗力倍率小提升”，不强制
}