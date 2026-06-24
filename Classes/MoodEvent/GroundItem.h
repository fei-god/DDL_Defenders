#pragma once

#include "cocos2d.h"
#include "GameObject.h"
#include "Player.h"

class GroundItem : public GameObject
{
public:
    virtual ~GroundItem() {}

    // ����ʰȡ�������������ִ��
    virtual void applyToPlayer(Player* player) = 0;

    // ������õ�����ʰȡ��ʧЧ���Ƴ�
    void onPicked(Player* player);
};