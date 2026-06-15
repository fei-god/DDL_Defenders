#pragma once
#ifndef __COFFEE_BLAST_H__
#define __COFFEE_BLAST_H__

#include "Weapon.h"

class CoffeeBlast : public Weapon
{
public:
    static CoffeeBlast* create(Player* owner);
    bool initCoffeeBlast(Player* owner);
    virtual void fire() override;

private:
    float _radius;
};

#endif
