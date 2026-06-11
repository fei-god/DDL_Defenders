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
    // 触发短时无敌：Player::setInvincible(true)（你当前 Player 无敌时长由 maxInvincibleTime 控制）
    player->setInvincible(true);
}