#pragma once
#ifndef __VICTORY_SCENE_H__
#define __VICTORY_SCENE_H__

#include "cocos2d.h"

class VictoryScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene(float survivalTime, int kills, int progress, int score);
    virtual bool init() override;

    void setResultData(float survivalTime, int kills, int progress, int score);
    void onRestartClicked(cocos2d::Ref* sender);
    void onTitleClicked(cocos2d::Ref* sender);

    CREATE_FUNC(VictoryScene);

private:
    float _survivalTime;
    int _kills;
    int _progress;
    int _score;
};

#endif
