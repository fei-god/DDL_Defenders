#pragma once
#ifndef __KEYBOARD_WAVE_H__
#define __KEYBOARD_WAVE_H__

#include "Weapon.h"

// 武器键盘冲击波特点：一次发射三发，范围比较宽。
class KeyboardWave : public Weapon
{
public:
    static KeyboardWave* create(Player* owner);

    bool initKeyboardWave(Player* owner);

    virtual void fire() override;

private:
    cocos2d::Vec2 rotateDirection(const cocos2d::Vec2& direction, float degree);
};

#endif