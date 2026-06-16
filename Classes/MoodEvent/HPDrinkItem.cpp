#include "HPDrinkItem.h"

USING_NS_CC;

HPDrinkItem* HPDrinkItem::create(const cocos2d::Vec2& pos, int healAmount, float focusDuration)
{
    auto item = new (std::nothrow) HPDrinkItem();
    if (item && item->initObject("hpdrink", GameObjectType::Item, "item_chocolate.png", pos))
    {
        item->_healAmount = healAmount;
        item->_focusDuration = focusDuration;
        item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return nullptr;
}

void HPDrinkItem::applyToPlayer(Player* player)
{
    if (!player) return;
    // ����/��������
    player->heal(_healAmount);
    // �ɽ�� Mood������ͼ��ɽ�� Mood��һ�£�
    player->changeMood(MoodType::Focus, _focusDuration);
}