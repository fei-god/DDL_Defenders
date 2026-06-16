#include "CoffeeBuffItem.h"

USING_NS_CC;

CoffeeBuffItem* CoffeeBuffItem::create(const Vec2& pos, float focusDuration)
{
    auto item = new (std::nothrow) CoffeeBuffItem();
    if (item && item->initObject(
        "coffee",
        GameObjectType::Item,
        "item_coffee.png", // <-- 改成你的咖啡图片文件名（或保持不变用做法A）
        pos))
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
    if (!player) return;
    player->changeMood(MoodType::Focus, _focusDuration);
}