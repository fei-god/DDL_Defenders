#pragma once
#ifndef COFFEE_GUN_H
#define COFFEE_GUN_H

#include "Weapon.h"

class CoffeeGun : public Weapon
{
public:
    static CoffeeGun* create(Player* owner);
    bool initCoffeeGun(Player* owner);
    virtual void fire() override;
};

#endif
