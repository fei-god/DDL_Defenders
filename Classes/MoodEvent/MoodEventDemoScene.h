#pragma once

#include "cocos2d.h"
#include "Player.h"

class MoodEventDemoScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override;

    CREATE_FUNC(MoodEventDemoScene);

private:
    void refreshHUD();

    void triggerMoveStart();  // ���� Focus
    void triggerMoveStop();
    void triggerHit(int damage); // ���� Irritable / Exhausted

private:
    Player* _player = nullptr;
    cocos2d::Label* _moodLabel = nullptr;
    cocos2d::Label* _hpLabel = nullptr;

    float _t = 0.0f;
    int _phase = 0; // �¼��ű��׶�
};