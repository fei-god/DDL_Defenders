#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;

    void onStartGameClicked(cocos2d::Ref* sender);
    void onLevelClicked(cocos2d::Ref* sender);
    void onEndlessClicked(cocos2d::Ref* sender);
    void onExitGameClicked(cocos2d::Ref* sender);
    void onSettingsClicked(cocos2d::Ref* sender);
    void onLeaderboardClicked(cocos2d::Ref* sender);

    CREATE_FUNC(MainMenuScene);
};

#endif // __MAIN_MENU_SCENE_H__
