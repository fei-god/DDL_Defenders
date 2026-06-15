#ifndef __GAME_OVER_SCENE_H__
#define __GAME_OVER_SCENE_H__

#include "cocos2d.h"

class GameOverScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene(float survivalTime);
    static cocos2d::Scene* createScene(float survivalTime, int kills, int progress, int score);

    virtual bool init() override;

    CREATE_FUNC(GameOverScene);

    void setSurvivalTime(float time);
    void setResultData(float survivalTime, int kills, int progress, int score);
    void onRestartClicked(cocos2d::Ref* sender);
    void onTitleClicked(cocos2d::Ref* sender);

private:
    float _survivalTime;
    int _kills;
    int _progress;
    int _score;
};

#endif // __GAME_OVER_SCENE_H__
