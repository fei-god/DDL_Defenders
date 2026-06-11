#pragma once

#include "cocos2d.h"
#include "Player.h"

class DDLTestScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(DDLTestScene);

private:
    void setupKeyboard();
    void updateDirectionFromKeys();
    void updateHUD();

    void onKeyPressed(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event);

    void menuCloseCallback(cocos2d::Ref* sender);

private:
    Player* _player = nullptr;
    cocos2d::Label* _moodLabel = nullptr;

    bool _keyLeft = false;
    bool _keyRight = false;
    bool _keyUp = false;
    bool _keyDown = false;
};