#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;

    void onStoryModeClicked(cocos2d::Ref* sender);
    void onEndlessClicked(cocos2d::Ref* sender);
    void onExitGameClicked(cocos2d::Ref* sender);
    void onSettingsClicked(cocos2d::Ref* sender);
    void onLeaderboardClicked(cocos2d::Ref* sender);

    CREATE_FUNC(MainMenuScene);

private:
    enum class MenuState { MAIN, TRANSITION, MENU };

    void startTransition();
    void animateButtons();

    MenuState _state = MenuState::MAIN;
    bool _canClickMenu = false;

    cocos2d::Sprite* _bgNormal = nullptr;
    cocos2d::Node* _bgBlur = nullptr;
    cocos2d::Node* _menuRoot = nullptr;

    cocos2d::MenuItemSprite* _btnStory = nullptr;
    cocos2d::MenuItemSprite* _btnEndless = nullptr;
    cocos2d::MenuItemSprite* _btnSettings = nullptr;
    cocos2d::MenuItemSprite* _btnExit = nullptr;

    float _s = 1.0f;
    cocos2d::Vec2 _btnTargetPos[4];
    cocos2d::Vec2 _btnStartPos[4];
};

#endif // __MAIN_MENU_SCENE_H__
