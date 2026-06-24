#include "ShieldScrollItem.h"

USING_NS_CC;

ShieldScrollItem* ShieldScrollItem::create(const cocos2d::Vec2& pos)
{
    auto item = new (std::nothrow) ShieldScrollItem();
    if (item && item->initObject("shield", GameObjectType::Item, "item_shield.png", pos))
    {
        item->autorelease();
        return item;
    }
    CC_SAFE_DELETE(item);
    return nullptr;
}

void ShieldScrollItem::applyToPlayer(Player* player)
{
    if (!player) return;
    // ������ʱ�޵У�Player::setInvincible(true)���㵱ǰ Player �޵�ʱ���� maxInvincibleTime ���ƣ�
    player->setInvincible(true);
}