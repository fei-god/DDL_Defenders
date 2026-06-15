#pragma once
#ifndef KEYBOARD_WEAP_H
#define KEYBOARD_WEAP_H

#include "Weapon.h"

class KeyboardWeap : public Weapon
{
public:
    static KeyboardWeap* create(Player* owner);
    bool initKeyboardWeap(Player* owner);
    virtual void fire() override;
};

#endif
