#pragma once
#ifndef __LEADERBOARD_SCENE_H__
#define __LEADERBOARD_SCENE_H__

#include "cocos2d.h"

class LeaderboardScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    void onBackClicked(cocos2d::Ref* sender);

    CREATE_FUNC(LeaderboardScene);
};

#endif
