#pragma once

#include "cocos2d.h"
#include "Player.h"

class RoommateAssistManager : public cocos2d::Ref
{
public:
    static RoommateAssistManager* create(Player* player);

    bool init(Player* player);

    void update(float dt);

private:
    void tryAssist();

private:
    Player* _player = nullptr;

    int _irritableTriggers = 0;
    bool _prevWasIrritable = false;
    bool _hasAssisted = false;
};