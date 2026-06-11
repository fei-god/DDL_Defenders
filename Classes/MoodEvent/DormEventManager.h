#pragma once

#include "cocos2d.h"
#include "Player.h"

class DormEventManager : public cocos2d::Ref
{
public:
    static DormEventManager* create(Player* player, cocos2d::Node* parentLayer, float intervalSeconds);

    bool init(Player* player, cocos2d::Node* parentLayer, float intervalSeconds);

    void update(float dt);

private:
    void triggerEvent(int eventType);

private:
    Player* _player = nullptr;
    cocos2d::Node* _parentLayer = nullptr;

    float _interval = 10.0f;
    float _timer = 0.0f;
};