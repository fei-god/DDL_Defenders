#pragma once

#include "Weapon.h"


class CoffeeLaser : public Weapon
{
public:
    static CoffeeLaser* create(Player* owner);

    bool initCoffeeLaser(Player* owner);

    virtual void fire() override;
};