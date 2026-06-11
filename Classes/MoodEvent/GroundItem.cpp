#include "GroundItem.h"

void GroundItem::onPicked(Player* player)
{
    if (player)
    {
        applyToPlayer(player);
    }

    // 标记失效并从父节点移除
    setActive(false);
    removeFromParent();
}
