#include "GroundItem.h"

void GroundItem::onPicked(Player* player)
{
    if (player)
    {
        applyToPlayer(player);
    }

    // ���ʧЧ���Ӹ��ڵ��Ƴ�
    setActive(false);
    removeFromParent();
}
